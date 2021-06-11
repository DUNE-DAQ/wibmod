#include <WIB/AddressTable.hh>
#include <fstream>
#include <WIB/AddressTableException.hh>
#include <boost/tokenizer.hpp> //tokenizer
#include <stdlib.h>  //strtoul & getenv
#include <boost/regex.hpp> //regex
#include <boost/algorithm/string/case_conv.hpp> //to_upper


uint32_t AddressTable::Read(uint16_t address){
  return io->Read(address);
}
uint32_t AddressTable::ReadWithRetry(uint16_t address){
  return io->ReadWithRetry(address);
}


void AddressTable::Write(uint16_t address, uint32_t data){
  io->Write(address,data);
}
void AddressTable::WriteWithRetry(uint16_t address, uint32_t data){
  io->WriteWithRetry(address,data);
}

void AddressTable::Write(uint16_t address, std::vector<uint32_t> const & values){
  io->Write(address,values);
}
void AddressTable::Write(uint16_t address,uint32_t const * values, size_t word_count){
  io->Write(address,values,word_count);
}


uint32_t AddressTable::Read(std::string registerName){
  std::map<std::string,Item *>::iterator itNameItem = nameItemMap.find(registerName);
  if(itNameItem == nameItemMap.end()){
    BUException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }
  Item * item = itNameItem->second;
  uint32_t val = io->Read(item->address);
  val &= (item->mask);
  val >>= item->offset;

  return val;
}

uint32_t AddressTable::ReadWithRetry(std::string registerName){
  std::map<std::string,Item *>::iterator itNameItem = nameItemMap.find(registerName);
  if(itNameItem == nameItemMap.end()){
    BUException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }
  Item * item = itNameItem->second;
  uint32_t val = io->ReadWithRetry(item->address);
  val &= (item->mask);
  val >>= item->offset;

  return val;
}

void AddressTable::Write(std::string registerName,uint32_t val){
  std::map<std::string,Item *>::iterator itNameItem = nameItemMap.find(registerName);
  if(itNameItem == nameItemMap.end()){
    BUException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }
  Item * item = itNameItem->second;
  //Check if this entry controls all the bits 
  uint32_t buildingVal =0;
  if(item->mask != 0xFFFFFFFF){
    //Since there are bits this register we don't control, we need to see what they currently are
    buildingVal = io->Read(item->address);
    buildingVal &= ~(item->mask);    
  }
  buildingVal |= (item->mask & (val << item->offset));
  io->Write(item->address,buildingVal);
}

void AddressTable::WriteWithRetry(std::string registerName,uint32_t val){
  std::map<std::string,Item *>::iterator itNameItem = nameItemMap.find(registerName);
  if(itNameItem == nameItemMap.end()){
    BUException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }
  Item * item = itNameItem->second;
  //Check if this entry controls all the bits 
  uint32_t buildingVal =0;
  if(item->mask != 0xFFFFFFFF){
    //Since there are bits this register we don't control, we need to see what they currently are
    buildingVal = io->ReadWithRetry(item->address);
    buildingVal &= ~(item->mask);    
  }
  buildingVal |= (item->mask & (val << item->offset));
  io->WriteWithRetry(item->address,buildingVal);
}


void AddressTable::Write(std::string registerName,std::vector<uint32_t> const & values){
  Write(registerName,values.data(),values.size());
}
void AddressTable::Write(std::string registerName,uint32_t const * values, size_t word_count){
  std::map<std::string,Item *>::iterator itNameItem = nameItemMap.find(registerName);
  if(itNameItem == nameItemMap.end()){
    BUException::INVALID_NAME e;
    e.Append("Can't find item with name \"");
    e.Append(registerName.c_str());
    e.Append("\"");
    throw e;    
  }
  Item * item = itNameItem->second;
  //Check if this entry controls all the bits 
  if(item->mask != 0xFFFFFFFF){
    BUException::BAD_BLOCK_WRITE e;
    e.Append("Mask is not 0xFFFFFFFF\n");
    throw e;
  }
  io->Write(item->address,values,word_count);
}

