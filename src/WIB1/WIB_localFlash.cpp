#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>

uint32_t WIB::ReadLocalFlash(uint16_t address){
  //load the address
  Write("SYSTEM.FLASH.ADDRESS",address);
  //start the read transaction
  Write("SYSTEM.FLASH.RW",1);
  Write("SYSTEM.FLASH.RUN",1);

  //Wait for transaction to finish
  while(Read("SYSTEM.FLASH.DONE") == 0){
    printf("busy\n");
    //sleep for 1ms
    usleep(1000);
  }
  
  return Read("SYSTEM.FLASH.RD_DATA");
}

std::vector<uint32_t> WIB::ReadLocalFlash(uint16_t address,size_t n){
  std::vector<uint32_t> readData;
  size_t current_address = address;
  size_t end_address = current_address + n;
  for(;current_address < end_address;current_address++){
    readData.push_back(ReadLocalFlash(current_address));
  }
  return readData;
}


void WIB::WriteLocalFlash(uint16_t address, uint32_t data){
  //load the address
  Write("SYSTEM.FLASH.ADDRESS",address);
  //load the data to write
  Write("SYSTEM.FLASH.WR_DATA",data);
  
  //start the read transaction
  Write("SYSTEM.FLASH.RW",0);
  Write("SYSTEM.FLASH.RUN",1);
  //Wait for finish
  while(Read("SYSTEM.FLASH.DONE") == 0){
    //sleep for 1ms
    usleep(1000);
  }
}


void WIB::WriteLocalFlash(uint16_t address,std::vector<uint32_t> const & data){
  for(size_t iWord = 0; iWord < data.size();iWord++){    
    WriteLocalFlash(address,data[iWord]);
    address++;
  }
}
