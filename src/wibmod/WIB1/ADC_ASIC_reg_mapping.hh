#ifndef __ADC_ASIC_REG_MAPPING_HH__
#define __ADC_ASIC_REG_MAPPING_HH__

#include <stdint.h>
#include <bitset>

class ADC_ASIC_reg_mapping {
 public:
  ADC_ASIC_reg_mapping();
  void set_ch(uint8_t chip=0, uint8_t chn=0, uint8_t d=0, uint8_t pcsr=0, 
         uint8_t pdsr=0, uint8_t slp=0, uint8_t tstin=0);
  void set_global(uint8_t chip, uint8_t f4=0, uint8_t f5=0, uint8_t slsb=0, 
                  uint8_t res4=0, uint8_t res3=0, uint8_t res2=0, 
                  uint8_t res1=0, uint8_t res0=0, uint8_t clk0=0, 
                  uint8_t clk1=0, uint8_t frqc=0, uint8_t engr=0, 
                  uint8_t f0=0, uint8_t f1=0, uint8_t f2=0, uint8_t f3=0);
  void set_chip(uint8_t chip=0, 
                  uint8_t d=0, uint8_t pcsr=0, uint8_t pdsr=0, 
                  uint8_t slp=0, uint8_t tstin=0, uint8_t f4=0, uint8_t f5=0, 
                  uint8_t slsb=0, uint8_t res4=0, uint8_t res3=0, 
                  uint8_t res2=0, uint8_t res1=0, uint8_t res0=0, 
                  uint8_t clk0=0, uint8_t clk1=0, uint8_t frqc=0, 
                  uint8_t engr=0, uint8_t f0=0, uint8_t f1=0, 
                  uint8_t f2=0, uint8_t f3=0);
  void set_board(uint8_t d=0, uint8_t pcsr=0, uint8_t pdsr=0, 
                  uint8_t slp=0, uint8_t tstin=0, uint8_t f4=0, uint8_t f5=0, 
                  uint8_t slsb=0, uint8_t res4=0, uint8_t res3=0, 
                  uint8_t res2=0, uint8_t res1=0, uint8_t res0=0, 
                  uint8_t clk0=0, uint8_t clk1=0, uint8_t frqc=0, 
                  uint8_t engr=0, uint8_t f0=0, uint8_t f1=0, 
                  uint8_t f2=0, uint8_t f3=0);
  std::bitset<1152> get_bits() const;
  void print() const;

 private:
  std::bitset<1152> BITS;
};
#endif
