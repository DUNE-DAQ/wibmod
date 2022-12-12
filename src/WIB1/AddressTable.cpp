#include "wibmod/WIB1/AddressTable.hh"
#include "wibmod/WIB1/AddressTableException.hh"
#include <boost/algorithm/string/case_conv.hpp> //to_upper
#include <boost/regex.hpp>                      //regex
#include <boost/tokenizer.hpp>                  //tokenizer
#include <fstream>
#include <stdlib.h> //strtoul & getenv

AddressTable::AddressTable(std::string const& addressTableName, std::string const& deviceAddress, uint16_t offset)
{
  fileLevel = 0;
  io = new BNL_UDP;
  io->Setup(deviceAddress, offset);
  io->SetWriteAck(true);
  LoadFile(addressTableName);
}
