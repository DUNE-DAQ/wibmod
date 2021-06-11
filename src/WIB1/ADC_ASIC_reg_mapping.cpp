#include <WIB/ADC_ASIC_reg_mapping.hh>
#include <iostream>

ADC_ASIC_reg_mapping::ADC_ASIC_reg_mapping(): BITS()
{
}

void ADC_ASIC_reg_mapping::set_ch(uint8_t chip, uint8_t chn, uint8_t d, uint8_t pcsr, 
       uint8_t pdsr, uint8_t slp, uint8_t tstin)
{
  uint8_t tmp_d = ((d<<4)&0xF0);
  uint8_t tmp_d0 = (tmp_d & 0x80)>>3;
  uint8_t tmp_d1 = (tmp_d & 0x40)>>1;
  uint8_t tmp_d2 = (tmp_d & 0x20)<<1;
  uint8_t tmp_d3 = (tmp_d & 0x10)<<3;
  tmp_d = tmp_d0 + tmp_d1 + tmp_d2 + tmp_d3;

  unsigned long chn_reg = (tmp_d&0xF0) + ((pcsr&0x01)<<3) + ((pdsr&0x01)<<2) + ((slp&0x01)<<1) + ((tstin&0x01)<<0);

  std::bitset<8> bits(chn_reg);
  size_t start_pos = (8*16+16)*chip + (16-chn)*8;
  for(size_t iBit=0; iBit < 8; iBit++)
  {
    BITS[iBit+start_pos-8] = bits[iBit];
  }
}
void ADC_ASIC_reg_mapping::set_global(uint8_t chip, uint8_t f4, uint8_t f5, uint8_t slsb, 
                uint8_t res4, uint8_t res3, uint8_t res2, 
                uint8_t res1, uint8_t res0, uint8_t clk0, 
                uint8_t clk1, uint8_t frqc, uint8_t engr, 
                uint8_t f0, uint8_t f1, uint8_t f2, uint8_t f3)
{
  std::bitset<16> bits;
  bits[ 0] = res0 & 0x1;
  bits[ 1] = res1 & 0x1;
  bits[ 2] = res2 & 0x1;
  bits[ 3] = res3 & 0x1;
  bits[ 4] = res4 & 0x1;
  bits[ 5] = slsb & 0x1;
  bits[ 6] = f5 & 0x1;
  bits[ 7] = f4 & 0x1;
  bits[ 8] = f3 & 0x1;
  bits[ 9] = f2 & 0x1;
  bits[10] = f1 & 0x1;
  bits[11] = f0 & 0x1;
  bits[12] = engr & 0x1;
  bits[13] = frqc & 0x1;
  bits[14] = clk1 & 0x1;
  bits[15] = clk0 & 0x1;

  size_t start_pos = (8*16+16)*chip + 16*8;
  for(size_t iBit=0; iBit < 16; iBit++)
  {
    BITS[iBit+start_pos] = bits[iBit];
  }
}
void ADC_ASIC_reg_mapping::set_chip(uint8_t chip, 
                uint8_t d, uint8_t pcsr, uint8_t pdsr, 
                uint8_t slp, uint8_t tstin, uint8_t f4, uint8_t f5, 
                uint8_t slsb, uint8_t res4, uint8_t res3, 
                uint8_t res2, uint8_t res1, uint8_t res0, 
                uint8_t clk0, uint8_t clk1, uint8_t frqc, 
                uint8_t engr, uint8_t f0, uint8_t f1, 
                uint8_t f2, uint8_t f3)
{
  for (size_t chn=0; chn<16; chn++)
  {
    set_ch(chip, chn, d, pcsr, pdsr, slp, tstin);
  }

  set_global( chip,
              f4, f5, slsb, res4, res3, res2, res1, res0,
              clk0, clk1, frqc, engr, f0, f1, f2, f3);
}

void ADC_ASIC_reg_mapping::set_board(uint8_t d, uint8_t pcsr, uint8_t pdsr, 
                uint8_t slp, uint8_t tstin, uint8_t f4, uint8_t f5, 
                uint8_t slsb, uint8_t res4, uint8_t res3, 
                uint8_t res2, uint8_t res1, uint8_t res0, 
                uint8_t clk0, uint8_t clk1, uint8_t frqc, 
                uint8_t engr, uint8_t f0, uint8_t f1, 
                uint8_t f2, uint8_t f3)
{

  for (size_t chip=0; chip<8; chip++)
  {
    set_chip(chip,
             d, pcsr, pdsr, slp, tstin,
             f4, f5, slsb, res4, res3, res2, res1, res0,
             clk0, clk1, frqc, engr, f0, f1, f2, f3);
  }
}

std::bitset<1152> ADC_ASIC_reg_mapping::get_bits() const
{
  return BITS;
}

void ADC_ASIC_reg_mapping::print() const
{
  std::cout << "ADC_ASIC_reg_mapping (binary):" << std::endl;
  std::string bitString = BITS.to_string<char,std::string::traits_type,std::string::allocator_type>();
  for(size_t iLine=0; iLine < 36; iLine++)
  {
    for(size_t iByte=0; iByte < 4; iByte++)
    {
      for(size_t iBit=0; iBit < 8; iBit++)
      {
        std::cout << bitString[iLine*32+iByte*8+iBit];
      }
      std::cout << ' ';
    }
    std::cout << std::endl;
  }
  //std::cout << BITS << std::endl;
}
