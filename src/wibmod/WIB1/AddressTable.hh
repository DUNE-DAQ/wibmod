#ifndef __ADDRESSTABLE_HPP__
#define __ADDRESSTABLE_HPP__

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <map>

#include <stdint.h>

#include <WIB/ItemConversion.hh>
#include <WIB/BNL_UDP.hh>

class Item{
public:
  enum ModeMask{EMPTY=0x0,READ = 0x1,WRITE =0x2, ACTION = 0x4};
  std::string name;
  uint16_t address;
  uint32_t mask;
  uint8_t  offset;
  uint8_t  mode; // r :0, w :1, a:2
  boost::unordered_map<std::string,std::string> user;   
  ItemConversion *sc_conv;
};

class AddressTable{
 public:
  AddressTable(std::string const & addressTableName, std::string const & deviceAddress,uint16_t offset);
  uint32_t Read(uint16_t);
  uint32_t Read(std::string registerName);
  uint32_t ReadWithRetry(uint16_t);
  uint32_t ReadWithRetry(std::string registerName);
  void Write(uint16_t, uint32_t);
  void Write(std::string registerName,uint32_t val);
  void WriteWithRetry(uint16_t, uint32_t);
  void WriteWithRetry(std::string registerName,uint32_t val);
  void Write(uint16_t, std::vector<uint32_t> const & values);
  void Write(std::string registerName,std::vector<uint32_t> const & values);
  void Write(uint16_t, uint32_t const * values, size_t word_count);
  void Write(std::string registerName, uint32_t const * values, size_t word_count);
  Item const * GetItem(std::string const &);
  std::vector<Item const *> GetTagged(std::string const & tag);
  std::vector<std::string> GetNames();
  std::vector<std::string> GetNames(std::string const &regex);
  std::vector<std::string> GetAddresses(uint16_t lower, uint16_t upper);
  std::string GetRemoteAddress(){return io->GetAddress();};
  std::vector<std::string> GetTables(std::string const &regex);
  uint64_t GetRetryCount(){return io->GetRetryCount();};
 private:  
  //default constructor is forbidden 
  AddressTable();
  //preventcopying
  AddressTable( const AddressTable & );
  AddressTable& operator=(const AddressTable &);

  int fileLevel;
  void LoadFile(std::string const &, std::string const & prefix = "",uint16_t offset=0);
  void ProcessLine(std::string const &,size_t,std::string const & prefix = "",uint16_t offset=0);
  //Ds of entries
  //req  req     req  req  tokenized
  //name address mask mode user
  void AddEntry(Item *);
  //Map of address to Items (master book-keeping; delete from here)
  std::map<uint32_t,std::vector<Item*> > addressItemMap;
  //Map of names to Items
  std::map<std::string,Item*> nameItemMap;

  BNL_UDP * io;
};
#endif
