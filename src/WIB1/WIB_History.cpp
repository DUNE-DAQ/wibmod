#include <WIB/WIB.hh>

std::vector<uint32_t> WIB::CaptureHistory(std::string const & address){
  std::vector<uint32_t> ret;
  uint32_t val;
  while( (val = Read(address)) & 0x1){
    ret.push_back(val);
  }
  return ret;
}

std::vector<uint128_t> WIB::CaptureHistory(std::string const & address,size_t wordCount){
  std::vector<uint128_t> ret;
  bool capture = true;
  uint16_t addr = GetItem(address)->address;
  while(capture){
    uint128_t val = 0;  
    //Read address last since it causes the incr.
    for(size_t offset = wordCount;offset > 0;offset--){
      val |= uint128_t(Read(addr+(offset-1)) << 32*(offset-1));
    }
    ret.push_back(val);
    if(!(val & 0x1)){
      break;
    }
  }
  return ret;
}
