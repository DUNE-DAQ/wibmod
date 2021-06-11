#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>
#include <unistd.h>

#include <stdio.h>


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
  
  sleep(1);
    
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

  sleep(1);
  //Enable the clock for FPGA
  WriteWithRetry("DTS.SI5344.ENABLE",1);
  sleep(1);

  //  char const * const PDTSStates[] = {"W_RST","W_SFP","W_CDR","W_ALIGN","W_FREQ","W_LOCK","W_RDY","0x7","RUN","0x9","0xA","0xB","ERR_R","ERR_T","0xE","0xF"};
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
  const int number_of_tries = 50;
  if(0 == clockSource){
    printf("\nSetup PDTS.\n");
    for(int tries = 1; tries <= number_of_tries;tries++){
      //Using PDTS, set that up.
      WriteWithRetry("DTS.PDTS_ENABLE",1);
      usleep(10000); //needed in new PDTS system to get to a good state before giving up and trying a new phase. 

      //See if we've locked
      uint32_t pdts_state = ReadWithRetry("DTS.PDTS_STATE");
      printf("Try % 3d.   PDTS state: %s (0x%01X)\n",tries,PDTSStates[pdts_state&0xF],pdts_state);

      if(0x6 == pdts_state){	

	//We are ready to align!	
	if(PDTSAlignment_timeout == 0){
	  //In this mode we exit here to allow the system to wait in 0x6 while we configure the rest of the system. 
	  //This requires FW 2018-10-09 to allow 0x6 to be a valid locked state.
	  printf("PDTS state: %s (0x%01X)\n",PDTSStates[pdts_state&0xF],pdts_state);
	  tries = number_of_tries+1; //break out completely	  
	}else{
	  //=======================================================================
	  //Wait for alignment to finish
	  bool waitingForAlignment = true;	
	  while(waitingForAlignment){
	    
	    //poll for the state to change
	    sleep(1);
	    uint32_t pdts_state = ReadWithRetry("DTS.PDTS_STATE");


	    if(0x8 == pdts_state){
	      printf("PDTS state: %s (0x%01X)\n",PDTSStates[pdts_state&0xF],pdts_state);
	      waitingForAlignment = false;
	      tries = number_of_tries+1; //break out completely
	    }else if((pdts_state < 0x6) || (pdts_state > 0x8)){
	      //Something went wrong, go back to trying. 
	      waitingForAlignment = false;
	      printf("Something went wrong!   PDTS state: %s (0x%01X)\n",PDTSStates[pdts_state&0xF],pdts_state);
	    
	      //We haven't locked, kick the SI5344 chip and see what happens
	      if(tries == number_of_tries){
		//We are DONE, fail.
		WriteWithRetry("DTS.PDTS_ENABLE",0);
		//Throw
		BUException::WIB_DTS_ERROR e;
		e.Append("Failed to configure the PDTS correctly\n");
		throw e;	  
	      }
	    
	      WriteWithRetry("DTS.PDTS_ENABLE",0);
	    
	      //dynamic post-amble
	      Write("DTS.SI5344.I2C.RESET",1);
	      SetDTS_SI5344Page(0x0);
	      Write("DTS.SI5344.I2C.RESET",1);
	      WriteDTS_SI5344(0x1C,0x1,1);
	      usleep(300000);
	    	    
	    }else{
	      printf("Waiting for phase alignment.   PDTS state: %s (0x%01X)\n",PDTSStates[pdts_state&0xF],pdts_state);	  
	      //timeout if requested
	      if(1 == PDTSAlignment_timeout){
		printf("Timeout in PDTS wait.   PDTS state: %s (0x%01X)\n",PDTSStates[pdts_state&0xF],pdts_state);	  
		//Throw
		BUException::WIB_DTS_ERROR e;
		e.Append("Timeout in the PDTS phase adjustment wait\n");
		throw e;	  	      
	      }else if(PDTSAlignment_timeout > 1){
		PDTSAlignment_timeout--;
	      }
	    }
	  }
	}
	//=======================================================================
      }else{
	//We haven't locked, kick the SI5344 chip and see what happens
	if(tries == number_of_tries){
	  //We are DONE, fail.
	  WriteWithRetry("DTS.PDTS_ENABLE",0);
	  //Throw
	  BUException::WIB_DTS_ERROR e;
	  e.Append("Failed to configure the PDTS correctly\n");
	  throw e;	  
	}

	WriteWithRetry("DTS.PDTS_ENABLE",0);



	//dynamic post-amble
        Write("DTS.SI5344.I2C.RESET",1);
        SetDTS_SI5344Page(0x0);
        Write("DTS.SI5344.I2C.RESET",1);
	WriteDTS_SI5344(0x1C,0x1,1);
	usleep(300000);
      }
    }
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
