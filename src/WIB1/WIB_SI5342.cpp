#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>
#include <fstream>
#include <unistd.h> //usleep

#define WIB_CONFIG_PATH "WIB_CONFIG_PATH" 
#define SI5342_CONFIG_FILENAME "FELIX_SI5342.txt"

void WIB::WriteDAQ_SI5342(uint16_t address,uint32_t value,uint8_t byte_count){
  WriteI2C("DAQ.SI5342.I2C",address,value,byte_count);
}
uint32_t WIB::ReadDAQ_SI5342(uint16_t address,uint8_t byte_count){
  return ReadI2C("DAQ.SI5342.I2C",address,byte_count);
}


void WIB::ResetSi5342(){
  Write("DAQ.SI5342.RESET",0x1);
  Write("DAQ.SI5342.RESET",0x0);
  usleep(100000);
}

void WIB::SetDAQ_SI5342Page(uint8_t page){
  WriteDAQ_SI5342(0x1,page,1);
}

uint8_t WIB::GetDAQ_SI5342Page(){
  return uint8_t(ReadDAQ_SI5342(0x1,1)&0xFF);
}

uint8_t WIB::GetDAQ_SI5342AddressPage(uint16_t address){
  return uint8_t((address >> 8)&0xFF); 
}

void WIB::LoadConfigDAQ_SI5342(std::string const & fileName){
  std::ifstream confFile(fileName.c_str());
  BUException::WIB_BAD_ARGS badFile;

  if(confFile.fail()){
    //Failed to topen filename, add it to the exception
    badFile.Append("Bad SI5342 config file name:");
    badFile.Append(fileName.c_str());

    //Try the default
    if(getenv(WIB_CONFIG_PATH) != NULL){      
      std::string envBasedFileName=getenv(WIB_CONFIG_PATH);
      envBasedFileName+="/";
      envBasedFileName+=SI5342_CONFIG_FILENAME;
      confFile.open(envBasedFileName.c_str());
      if(confFile.fail()){
	badFile.Append("Bad env based filename:");
	badFile.Append(envBasedFileName.c_str());
      }
    }
  }
  
  if(confFile.fail()){
    //We are still failing to open our file
    throw badFile;
  }

  //Make sure the chip isn't in reset
  if(Read("DAQ.SI5342.RESET") != 0){
    Write("DAQ.SI5342.RESET",0x0);
    usleep(50000);
  }

  //Reset the I2C firmware
  Write("DAQ.SI5342.I2C.RESET",1);

  std::vector<std::pair<uint16_t,uint8_t> > writes;
  while(!confFile.eof()){
    std::string line;
    std::getline(confFile,line);
    if(line.size() == 0){
      continue;
    }else if(line[0] == '#'){
      continue;
    }else if(line[0] == 'A'){
      continue;
    }else{
      if( line.find(',') == std::string::npos ){
	printf("Skipping bad line: \"%s\"\n",line.c_str());
	continue;
      }
      uint16_t address = strtoul(line.substr(0,line.find(',')).c_str(),NULL,16);
      uint8_t  data    = strtoul(line.substr(line.find(',')+1).c_str(),NULL,16);
      writes.push_back(std::pair<uint16_t,uint8_t>(address,data));
    }
  }

  //Disable the SI5342 output
  Write("DAQ.SI5342.ENABLE",0x0);

  uint8_t page = GetDAQ_SI5342Page();
  unsigned int percentDone = 0;


  printf("\n[==================================================]\n");
  fprintf(stderr," ");
  for(size_t iWrite = 0; iWrite < writes.size();iWrite++){

    if(page != GetDAQ_SI5342AddressPage(writes[iWrite].first)){
      page = GetDAQ_SI5342AddressPage(writes[iWrite].first);
      SetDAQ_SI5342Page(page);
      usleep(100000);
    }

    
    if(iWrite == 3){
      usleep(300000);
    }
    

    uint8_t  address = writes[iWrite].first & 0xFF;
    uint32_t data = (writes[iWrite].second) & 0xFF;
    uint8_t  iData = 1;

    for(size_t iTries = 10; iTries > 0;iTries--){
      try{
	WriteDAQ_SI5342(address ,data,iData);
      }catch (BUException::WIB_ERROR & e){	
	//Reset the I2C firmware
	Write("DAQ.SI5342.I2C.RESET",1);
	if(iTries == 1){
	  e.Append("\nTried 3 times\n");
	  throw;
	}
      }
    }
    if((100*iWrite)/writes.size() > percentDone){
      fprintf(stderr,"#");
      percentDone+=2;
    }
  }
  printf("\n");

}

void WIB::SelectSI5342(uint64_t input,bool enable){
  Write("DAQ.SI5342.INPUT_SELECT", input); 
  Write("DAQ.SI5342.ENABLE", uint64_t(enable)); 
}
