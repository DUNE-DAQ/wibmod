#ifndef __WIBBASE_HH__
#define __WIBBASE_HH__

#include <string>
#include <stdint.h>

#include <WIB/AddressTable.hh>

#define FEMB_COUNT 4

class WIBBase {
public:
  WIBBase(std::string const & address, std::string const & WIBAddressTable, std::string const & FEMBAddressTable);
  ~WIBBase();

  std::string GetAddress();

  uint32_t Read(uint16_t address);
  uint32_t ReadWithRetry(uint16_t address);
  uint32_t Read(std::string const & address);
  uint32_t ReadWithRetry(std::string const & address);
  void Write(uint16_t address,uint32_t value);
  void WriteWithRetry(uint16_t address,uint32_t value);
  void Write(std::string const & address,uint32_t value);
  void WriteWithRetry(std::string const & address,uint32_t value);
  void Write(uint16_t address,std::vector<uint32_t> const & values);
  void Write(std::string const & address,std::vector<uint32_t> const & values);
  void Write(uint16_t address,uint32_t const * values,size_t word_count);
  void Write(std::string const & address,uint32_t const * values,size_t word_count);


  uint32_t ReadI2C(std::string const & base_address,uint16_t I2C_aaddress, uint8_t byte_count=4);
  void     WriteI2C(std::string const & base_address,uint16_t I2C_address, uint32_t data, uint8_t byte_count=4,bool ignore_error = false);

  std::vector<std::string> GetNames(std::string const & regex){
    return wib->GetNames(regex);
  }
  std::vector<std::string> GetFEMBNames(std::string const & regex){
    return FEMB[0]->GetNames(regex);
  }
  std::vector<std::string> GetAddresses(uint16_t lower,uint16_t upper){
    return wib->GetAddresses(lower,upper);
  }
  std::vector<std::string> GetFEMBAddresses(uint16_t lower,uint16_t upper){
    return FEMB[0]->GetAddresses(lower,upper);
  }
  std::vector<std::string> GetTableNames(std::string const & regex){
    return wib->GetTables(regex);
  }

  std::vector<Item const *> GetTagged(std::string const & tag) {
    return wib->GetTagged(tag);
  }

  std::vector<Item const *> GetFEMBTagged(std::string const & tag) {
    return FEMB[0]->GetTagged(tag);
  }


  uint32_t ReadFEMB(int iFEMB, uint16_t address);
  uint32_t ReadFEMB(int iFEMB, std::string const & address);
  void WriteFEMB(int iFEMB, uint16_t address, uint32_t value);
  void WriteFEMB(int iFEMB, std::string const & address, uint32_t value);
  void WriteFEMBBits(int iFEMB, uint16_t address, uint32_t pos, uint32_t mask, uint32_t value);
  void EnableADC(uint64_t iFEMB, uint64_t enable);

  Item const * GetItem(std::string const &);
  Item const * GetFEMBItem(int iFEMB,std::string const &);

  int GetSVNVersion(){return Version;}
private:  
  WIBBase(); //disallow the default constructor
  // Prevent copying of WIB objects
  WIBBase( const WIBBase& other) ; // prevents construction-copy
  WIBBase& operator=( const WIBBase&) ; // prevents copying
  
  AddressTable * wib;
  AddressTable * FEMB[FEMB_COUNT];
  static const int Version; //SVN version
  const float FEMBReadSleepTime;
  const float FEMBWriteSleepTime;
};
#endif
