#ifndef ITEMCONVERSION_HH_
#define ITEMCONVERSION_HH_

#include <string>
#include <boost/unordered_map.hpp>

class ItemConversion {
protected:
  size_t data_size;
  std::string data_description;
  ItemConversion(){}
  ItemConversion
    (boost::unordered_map<std::string, std::string> ) { }
public:
  virtual ~ItemConversion() { }
  size_t DataSize() {
    return data_size;
  }
  std::string DataDescription() {
    return data_description;
  }
  virtual void Convert(uint32_t src, void *dest) = 0;
  static ItemConversion * FromString(std::string convstring);



};

#endif
