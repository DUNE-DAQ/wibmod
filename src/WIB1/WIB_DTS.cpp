#include "wibmod/WIB1/WIB.hh"
#include "wibmod/Issues.hpp"
#include "wibmod/WIB1/WIBException.hh"
#include "ers/ers.hpp"

#include <unistd.h>

#include <stdio.h>
#include <chrono>


void WIB::InitializeDTS(uint8_t PDTSsource,uint8_t clockSource, uint32_t PDTSAlignment_timeout){
  //Disable the PDTS
  WriteWithRetry("DTS.PDTS_ENABLE",0x0);

  //Disable SI5344 outputs (fixed by SI5344 config)
  WriteWithRetry("DTS.SI5344.ENABLE",0);
  //Disable SI5344 (fixed by SI5344 config)
  WriteWithRetry("DTS.SI5344.RESET",1);


  //Reset the I2C firmware
  WriteWithRetry("DTS.CDS.I2C.RESET",1);

  
  if(0 == clockSource){
    printf("Using PDTS for DUNE timing.\n\nConfiguring clock and data separator\n");
    //Bring up the CDS
    float frequency = 0;
    try{
      frequency = ConfigureDTSCDS(PDTSsource);
    }catch(BUException::exBase & e){
      frequency = 0;
      e.Append("Failed to communicate with the DTS CDS via I2C\n");
      throw;
    }
    uint32_t LOL = Read("DTS.CDS.LOL");
    uint32_t LOS = Read("DTS.CDS.LOS");    
    printf("CDS frequency %f\n",frequency);
    printf("CDS LOL=%d LOS=%d\n",LOL,LOS);
  
    //Check for the correct frequency, not in LOS, and not in LOL
    //    if( (2.4136e+08 == frequency) && 
    if(LOL || LOS){
      BUException::WIB_DTS_ERROR e;
      e.Append("Failed to configure CDS chip\n");
      throw e;
    }
  }else{
    printf("Using local OSC for DUNE timing\n");
  }
  //CDS is up


  printf("\nConfiguring SI5344.\n");

  //Set the SI5344 source
  WriteWithRetry("DTS.SI5344.INPUT_SELECT",clockSource);
  //Configure Si5344 with default config file
  //Do the I2C configuration
  try{
    LoadConfigDTS_SI5344(""); 
  }catch(BUException::exBase & e){
    //Disable SI5344 outputs
    WriteWithRetry("DTS.SI5344.ENABLE",0);
    //Disable SI5344
    WriteWithRetry("DTS.SI5344.RESET",1);	
    
    e.Append("Error in LoadConfigDTS_SI5344\n");
    throw;
  }
  
  usleep(100000);
    
  //Check that SI5344 is locked on
  if(ReadWithRetry("DTS.SI5344.LOS") ||
     ReadWithRetry("DTS.SI5344.LOL")){
    //Disable SI5344 outputs
    WriteWithRetry("DTS.SI5344.ENABLE",0);
    //Disable SI5344
    WriteWithRetry("DTS.SI5344.RESET",1);	
    
    //Throw
    BUException::WIB_DTS_ERROR e;
    e.Append("Failed to configure the SI5344 chip correctly\n");
    throw e;
  }

  //Enable the clock for FPGA
  WriteWithRetry("DTS.SI5344.ENABLE",1);
  usleep(100000);

  char const * const PDTSStates[] = {"W_RST",
				     "W_LINK",
				     "W_FREQ",
				     "W_ADJUST",
				     "W_ALIGN",
				     "W_LOCK",
				     "W_PHASE",
				     "W_RDY",
				     "RUN",
				     "0x9",
				     "0xA",
				     "0xB",
				     "ERR_R",
				     "ERR_T",
				     "ERR_P",
				     "0xF"};

  
  if(0 == clockSource){
    printf("\nSetup PDTS.\n");
    bool timeout_exists = true;
    if (PDTSAlignment_timeout == 0)
	timeout_exists = false;

    auto start_time = std::chrono::high_resolution_clock::now();
    bool timed_out = false;

    while ((timeout_exists == false) || timed_out == false) {
      //Using PDTS, set that up.
      usleep(500000);
      WriteWithRetry("DTS.PDTS_ENABLE",1);
      usleep(500000); //needed in new PDTS system to get to a good state before giving up and trying a new phase. 

      //See if we've locked
      uint32_t pdts_state = ReadWithRetry("DTS.PDTS_STATE");
      printf("PDTS state: %s (0x%01X)\n",PDTSStates[pdts_state&0xF],pdts_state);
      if ((pdts_state < 0x6) || (pdts_state > 0x8)) {
          WriteWithRetry("DTS.PDTS_ENABLE",0);
          //dynamic post-amble
          Write("DTS.SI5344.I2C.RESET",1);
          SetDTS_SI5344Page(0x0);
          Write("DTS.SI5344.I2C.RESET",1);
          WriteDTS_SI5344(0x1C,0x1,1);
      }
      else if(0x6 == pdts_state || 0x7 == pdts_state){
          ers::info(dunedaq::wibmod::WaitingForAlignment(ERS_HERE));
      }
      else {
          return; //0x8 == pdts_state
      }
      auto now = std::chrono::high_resolution_clock::now();
      auto duration = now - start_time;
      if ( duration.count() > PDTSAlignment_timeout)
          timed_out = true;
    }
    //If we get here something went wrong
    Write("DTS.SI5344.I2C.RESET",1);
    SetDTS_SI5344Page(0x0);
    Write("DTS.SI5344.I2C.RESET",1);
    WriteDTS_SI5344(0x1C,0x1,1);

    BUException::WIB_DTS_ERROR e;
    e.Append("Failed to configure the PDTS correctly within timeout\n");
    throw e; 
  }
}

void WIB::StartSyncDTS(){
  WriteWithRetry("DTS.CONVERT_CONTROL.HALT",0);
  WriteWithRetry("DTS.CONVERT_CONTROL.ENABLE",1);
  WriteWithRetry("DTS.CONVERT_CONTROL.START_SYNC",1);
}

void WIB::PDTSInRunningState(){
  if(Read("DTS.PDTS_STATE") != 0x8){
    BUException::WIB_DTS_ERROR e;
    e.Append("WIB is not in PDTS state RUN(0x8)\n");
    throw e;
  }
}
