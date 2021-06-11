#ifndef __ASIC_REG_MAPPING_HH__
#define __ASIC_REG_MAPPING_HH__

#include <stdint.h>
#include <vector>
#include <WIB/ADC_ASIC_reg_mapping.hh>
#include <WIB/FE_ASIC_reg_mapping.hh>

class ASIC_reg_mapping {
 public:
  ASIC_reg_mapping();
  void set_board(const FE_ASIC_reg_mapping & fe_map, const ADC_ASIC_reg_mapping & adc_map);
  std::vector<uint32_t> get_regs() const;
  void print() const;

 private:
  std::vector<uint32_t> REGS;
};
#endif
