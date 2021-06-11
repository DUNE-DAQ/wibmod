#include <WIB/WIBBase.hh>
#include <WIB/WIBException.hh>
#include <WIB/BNL_UDP_Exception.hh>

WIBBase::WIBBase(std::string const & address, std::string const & WIBAddressTable, std::string const & FEMBAddressTable):
  wib(NULL), FEMBReadSleepTime(0.01), FEMBWriteSleepTime(0.01) {
  //Make sure all pointers and NULL before any allocation

  for(size_t iFEMB = 0; iFEMB < FEMB_COUNT;iFEMB++){
    FEMB[iFEMB] = NULL;
  }

  //Create the wib address table interface
  wib = new AddressTable(WIBAddressTable,address,0);
  for(size_t iFEMB = 0; iFEMB < FEMB_COUNT;iFEMB++){
    FEMB[iFEMB] = new AddressTable(FEMBAddressTable,address,(iFEMB+1)*0x10);
  }
}

WIBBase::~WIBBase(){
  if(wib != NULL){
    delete wib;
    wib = NULL;
  }
  for(size_t iFEMB = 0; iFEMB < FEMB_COUNT;iFEMB++){
    if(FEMB[iFEMB] != NULL){
      delete FEMB[iFEMB];
      FEMB[iFEMB] = NULL;
    }
  }
}


std::string WIBBase::GetAddress(){
  return wib->GetRemoteAddress();  
}


uint32_t WIBBase::ReadI2C(std::string const & base_address ,uint16_t address, uint8_t byte_count){
  //This is an incredibly inefficient version of this function since it does a dozen or so UDP transactions.
  //This is done to be generic, but it could be hard-coded by assuming address offsets and bit maps and done in one write and one read.

   //Set type of read
  WriteWithRetry(base_address+".RW",1);
   //Set address
  WriteWithRetry(base_address+".ADDR",address);
  //Set read size
  WriteWithRetry(base_address+".BYTE_COUNT",byte_count);
  //Wait for the last transaction to be done
  while(ReadWithRetry(base_address+".DONE") == 0x0){usleep(1000);}
  //Run transaction
  WriteWithRetry(base_address+".RUN",0x1);
  //Wait for the last transaction to be done
  while(ReadWithRetry(base_address+".DONE") == 0x0){usleep(1000);}
  if(ReadWithRetry(base_address+".ERROR")){
    printf("%s 0x%08X\n",(base_address+".ERROR").c_str(),ReadWithRetry(base_address+".ERROR"));
    //Reset the I2C firmware
    WriteWithRetry(base_address+".RESET",1);    
    char trans_info[] = "rd @ 0xFFFF";
    sprintf(trans_info,"rd @ 0x%04X",address&0xFFFF);
    BUException::WIB_ERROR e;
    e.Append("I2C Error on ");
    e.Append(base_address.c_str());
    e.Append(trans_info);
    throw e;
  }
  return ReadWithRetry(base_address+".RD_DATA");
}
void     WIBBase::WriteI2C(std::string const & base_address,uint16_t address, uint32_t data, uint8_t byte_count,bool ignore_error){
  //This is an incredibly inefficient version of this function since it does a dozen or so UDP transactions.
  //This is done to be generic, but it could be hard-coded by assuming address offsets and bit maps and done in one write and one read.

   //Set type of read
  WriteWithRetry(base_address+".RW",0);
  //Set address
  WriteWithRetry(base_address+".ADDR",address);
  //Set read size
  WriteWithRetry(base_address+".BYTE_COUNT",byte_count);
  //Send data to write
  WriteWithRetry(base_address+".WR_DATA",data);
  //Wait for the last transaction to be done
  while(ReadWithRetry(base_address+".DONE") == 0x0){usleep(1000);}
  //Run transaction
  WriteWithRetry(base_address+".RUN",0x1);

  //Wait for the last transaction to be done
  while(ReadWithRetry(base_address+".DONE") == 0x0){usleep(1000);}

  if(!ignore_error && ReadWithRetry(base_address+".ERROR")){
    //Reset the I2C firmware
    WriteWithRetry(base_address+".RESET",1);    
    char trans_info[] = "wr 0xFFFFFFFF @ 0xFFFF";
    sprintf(trans_info,"wr 0x%08X @ 0x%04X",data,address&0xFFFF);
    BUException::WIB_ERROR e;
    e.Append("I2C Error on ");
    e.Append(base_address.c_str());
    e.Append(trans_info);
    throw e;
  }
}


Item const * WIBBase::GetItem(std::string const & str){
  return wib->GetItem(str);
}

Item const * WIBBase::GetFEMBItem(int iFEMB,std::string const & str){
  if((iFEMB > 4) || (iFEMB <1)){
    BUException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("In WIBBase::ReadFEMB\n");
    throw e;
  }
  return FEMB[iFEMB-1]->GetItem(str);
}

uint32_t WIBBase::ReadWithRetry(uint16_t address){
  return wib->ReadWithRetry(address);    
}
uint32_t WIBBase::Read(uint16_t address){
  return wib->Read(address);    
}
uint32_t WIBBase::ReadWithRetry(std::string const & address){
  return wib->ReadWithRetry(address);    
}
uint32_t WIBBase::Read(std::string const & address){
  return wib->Read(address);    
}

void WIBBase::WriteWithRetry(uint16_t address,uint32_t value){
  wib->WriteWithRetry(address,value);    
}
void WIBBase::Write(uint16_t address,uint32_t value){
  wib->Write(address,value);    
}
void WIBBase::WriteWithRetry(std::string const & address,uint32_t value){
  wib->WriteWithRetry(address,value);    
}
void WIBBase::Write(std::string const & address,uint32_t value){
  wib->Write(address,value);    
}
void WIBBase::Write(uint16_t address,std::vector<uint32_t> const & values){
  wib->Write(address,values);    
}
void WIBBase::Write(std::string const & address,std::vector<uint32_t> const & values){
  wib->Write(address,values);    
}
void WIBBase::Write(uint16_t address,uint32_t const * values,size_t word_count){
  wib->Write(address,values,word_count);    
}
void WIBBase::Write(std::string const & address,uint32_t const * values,size_t word_count){
  wib->Write(address,values,word_count);    
}


uint32_t WIBBase::ReadFEMB(int iFEMB,uint16_t address){
  if((iFEMB > 4) || (iFEMB <1)){
    BUException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("In WIBBase::ReadFEMB\n");
    throw e;
  }
  return FEMB[iFEMB-1]->Read(address);    
  usleep((useconds_t) FEMBReadSleepTime * 1e6);
}
uint32_t WIBBase::ReadFEMB(int iFEMB,std::string const & address){
  if((iFEMB > 4) || (iFEMB <1)){
    BUException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("In WIBBase::ReadFEMB\n");
    throw e;
  }
  return FEMB[iFEMB-1]->Read(address);    
  usleep((useconds_t) FEMBReadSleepTime * 1e6);
}

void WIBBase::WriteFEMB(int iFEMB,uint16_t address,uint32_t value){
  if((iFEMB > 4) || (iFEMB <1)){
    BUException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("In WIBBase::WriteFEMB\n");
    throw e;
  }
  FEMB[iFEMB-1]->WriteWithRetry(address,value);    
  usleep((useconds_t) FEMBWriteSleepTime * 1e6);
}
void WIBBase::WriteFEMB(int iFEMB,std::string const & address,uint32_t value){
  if((iFEMB > 4) || (iFEMB <1)){
    BUException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("In WIBBase::WriteFEMB\n");
    throw e;
  }
  FEMB[iFEMB-1]->WriteWithRetry(address,value);    
  usleep((useconds_t) FEMBWriteSleepTime * 1e6);
}

void WIBBase::WriteFEMBBits(int iFEMB, uint16_t address, uint32_t pos, uint32_t mask, uint32_t value){
  if((iFEMB > 4) || (iFEMB <1)){
    BUException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("In WIBBase::WriteFEMB\n");
    throw e;
  }

  uint32_t shiftVal = value & mask;
  uint32_t regMask = (mask << pos);
  uint32_t initVal = FEMB[iFEMB -1]->Read(address);
  uint32_t newVal = ( (initVal & ~(regMask)) | (shiftVal << pos) );

  FEMB[iFEMB -1]->WriteWithRetry(address,newVal);
  usleep((useconds_t) FEMBWriteSleepTime * 1e6);
}

void WIBBase::EnableADC(uint64_t iFEMB, uint64_t enable){
  if(iFEMB > 4 || iFEMB < 1){
    BUException::WIB_INDEX_OUT_OF_RANGE e;
    e.Append("In WIBBase::Enable_ADC");
    throw e;
  }
  if(bool(enable))FEMB[iFEMB-1]->Write(0x03,0x00);
  else FEMB[iFEMB-1]->Write(0x03,0xFF);
  usleep((useconds_t) FEMBWriteSleepTime * 1e6);
}
