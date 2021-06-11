#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>
#include <fstream>
#include <unistd.h> //usleep


void WIB::WriteQSFP(uint16_t address,uint32_t value,uint8_t byte_count){
  WriteI2C("DAQ.QSFP.I2C",address,value,byte_count);
}
uint32_t WIB::ReadQSFP(uint16_t address,uint8_t byte_count){
  return ReadI2C("DAQ.QSFP.I2C",address,byte_count);
}


