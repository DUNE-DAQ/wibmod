#include <WIB/FE_ASIC_reg_mapping.hh>
#include <iostream>

FE_ASIC_reg_mapping::FE_ASIC_reg_mapping(): BITS()
{
}

void FE_ASIC_reg_mapping::set_ch(uint8_t chip, uint8_t chn, uint8_t sts, uint8_t snc, 
           uint8_t sg, uint8_t st, uint8_t smn, uint8_t sdf)
{
  unsigned long chn_reg = ((sts&0x01)<<7) + ((snc&0x01)<<6) + ((sg&0x03)<<4) 
                    + ((st&0x03)<<2)  + ((smn&0x01)<<1) + ((sdf&0x01)<<0);
  std::bitset<8> bits(chn_reg);
  size_t start_pos = (8*16+16)*chip + (16-chn)*8;
  for(size_t iBit=0; iBit < 8; iBit++)
  {
    BITS[iBit+start_pos-8] = bits[iBit];
  }
}
void FE_ASIC_reg_mapping::set_global(uint8_t chip, uint8_t slk0, uint8_t stb1,
           uint8_t stb, uint8_t s16, uint8_t slk1, 
           uint8_t sdc, uint8_t swdac, uint8_t dac)
{
  unsigned long global_reg = ((slk0&0x01)<<0) + ((stb1&0x01)<<1) + ((stb&0x01)<<2) 
               + ((s16&0x01)<<3) + ((slk1&0x01)<<4) + ((sdc&0x01)<<5) +((0&0x03)<<6);

  unsigned long dac_reg = (((dac&0x01)/0x01)<<7)+(((dac&0x02)/0x02)<<6)
            +(((dac&0x04)/0x04)<<5)+(((dac&0x08)/0x08)<<4)
            +(((dac&0x10)/0x10)<<3)+(((dac&0x20)/0x20)<<2)
            +(((swdac&0x03))<<0);

  std::bitset<8> global_bits(global_reg);
  std::bitset<8> dac_bits(dac_reg);

  std::bitset<16> bits;
  for(size_t iBit=0; iBit < 8; iBit++)
  {
    bits[iBit] = global_bits[iBit];
    bits[iBit+8] = dac_bits[iBit];
  }

  size_t start_pos = (8*16+16)*chip + 16*8;
  for(size_t iBit=0; iBit < 16; iBit++)
  {
    BITS[iBit+start_pos] = bits[iBit];
  }
}

void FE_ASIC_reg_mapping::set_chip(uint8_t chip, 
         uint8_t sts, uint8_t snc, uint8_t sg, uint8_t st, 
         uint8_t smn, uint8_t sdf, uint8_t slk0, 
         uint8_t stb1, uint8_t stb, uint8_t s16, 
         uint8_t slk1, uint8_t sdc, uint8_t swdac, uint8_t dac)
{
  for (size_t chn=0; chn<16; chn++)
  {
      set_ch(chip, chn, sts, snc, sg, st, smn, sdf);
  }
  set_global (chip, slk0, stb1, stb, s16, slk1, sdc, swdac, dac);
}

void FE_ASIC_reg_mapping::set_board(uint8_t sts, uint8_t snc, uint8_t sg, uint8_t st, 
         uint8_t smn, uint8_t sdf, uint8_t slk0, 
         uint8_t stb1, uint8_t stb, uint8_t s16, 
         uint8_t slk1, uint8_t sdc, uint8_t swdac, uint8_t dac)
{
  for (size_t chip=0; chip<8; chip++)
  {
     set_chip( chip, sts, snc, sg, st, smn, sdf, slk0, stb1, stb, s16, slk1, sdc, swdac, dac);
  }
}

std::bitset<1152> FE_ASIC_reg_mapping::get_bits() const
{
  return BITS;
}


void FE_ASIC_reg_mapping::set_collection_baseline(uint8_t snc)
{
  for (size_t chip=0; chip<8; chip++)
  {
    for (size_t chn=0; chn<16; chn++)
    {
      bool isCollection = (channel_wire_plane[chip][chn] == 2);
      size_t start_pos = (8*16+16)*chip + (16-chn)*8;
      if (isCollection)
      {
        BITS[6+start_pos-8] = snc & 0x1;
      }
    }
  }
}

void FE_ASIC_reg_mapping::print() const
{
  std::cout << "FE_ASIC_reg_mapping (binary):" << std::endl;
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

// wire plane [asic 0-7][channel 0-15] 
// U=0, V=1, W/X/Z/collection=2; increasing in direction electrons drift as larsoft does
const uint8_t FE_ASIC_reg_mapping::channel_wire_plane[8][16] = {
  {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
  {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
  {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
  {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
  {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
  {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
  {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0}
};
