#ifndef __FE_ASIC_REG_MAPPING_HH__
#define __FE_ASIC_REG_MAPPING_HH__

#include <stdint.h>
#include <bitset>

class FE_ASIC_reg_mapping {
 public:
  FE_ASIC_reg_mapping();
  void set_ch(uint8_t chip=0, uint8_t chn=0, uint8_t sts=0, uint8_t snc=0, 
             uint8_t sg=0, uint8_t st=0, uint8_t smn=0, uint8_t sdf=0);
  void set_global(uint8_t chip=0, uint8_t slk0 = 0, uint8_t stb1 = 0,
             uint8_t stb = 0, uint8_t s16=0, uint8_t slk1=0, 
             uint8_t sdc = 0, uint8_t swdac=0, uint8_t dac=0);
  void set_chip(uint8_t chip=0, 
           uint8_t sts=0, uint8_t snc=0, uint8_t sg=0, uint8_t st=0, 
           uint8_t smn=0, uint8_t sdf=0, uint8_t slk0=0, 
           uint8_t stb1=0, uint8_t stb=0, uint8_t s16=0, 
           uint8_t slk1=0, uint8_t sdc=0, uint8_t swdac=0, uint8_t dac=0);

  void set_board(uint8_t sts=0, uint8_t snc=0, uint8_t sg=0, uint8_t st=0, 
           uint8_t smn=0, uint8_t sdf=0, uint8_t slk0=0, 
           uint8_t stb1=0, uint8_t stb=0, uint8_t s16=0, 
           uint8_t slk1=0, uint8_t sdc=0, uint8_t swdac=0, uint8_t dac=0);
  std::bitset<1152> get_bits() const;
  void set_collection_baseline(uint8_t snc);
  void print() const;

 private:
  std::bitset<1152> BITS;
  static const uint8_t channel_wire_plane[8][16];
};
#endif
