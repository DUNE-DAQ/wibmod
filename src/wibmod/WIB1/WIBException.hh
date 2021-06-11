#ifndef __WIB_EXCEPTION_HH__
#define __WIB_EXCEPTION_HH__ 1

#include <BUException/ExceptionBase.hh>

namespace BUException{       
  //Exceptions for WIB
  ExceptionClassGenerator(WIB_INDEX_OUT_OF_RANGE,"Index out of range\n")
  //  ExceptionClassGenerator(WIB_FEMB_RANGE,"WIB FEMB out of range\n")
  //  ExceptionClassGenerator(WIB_DAQ_LINK_RANGE,"WIB DAQ Link out of range\n")
  ExceptionClassGenerator(WIB_BUSY,"WIB BUSY\n")
  ExceptionClassGenerator(WIB_FEATURE_NOT_SUPPORTED,"Requested feature is not available\n")
  ExceptionClassGenerator(WIB_BAD_ARGS,"WIB bad args")
  ExceptionClassGenerator(WIB_ERROR,"WIB Error")
  ExceptionClassGenerator(WIB_DAQMODE_UNKNOWN,"Unknown whether WIB is in RCE or FELIX mode\n")
  ExceptionClassGenerator(WIB_FAKE_DATA_ON_HALF_FELIX_LINK,"Fake data mode set on only 1 FEMB of a FELIX link--not allowed")
  ExceptionClassGenerator(WIB_FLASH_TIMEOUT,"A timeout occured on WIB flash")
  ExceptionClassGenerator(WIB_FLASH_ERROR,"An error while using the flash")
  ExceptionClassGenerator(WIB_FLASH_IHEX_ERROR,"An error while parsing intel Hex files")
  ExceptionClassGenerator(WIB_DTS_ERROR,"WIB timing system error")
  ExceptionClassGenerator(FEMB_FIRMWARE_VERSION_MISMATCH,"FEMB firmware version mismatch")
  ExceptionClassGenerator(FEMB_REG_READ_ERROR,"Can't read FEMB registers")
  ExceptionClassGenerator(FEMB_SPI_READBACK_MISMATCH,"FEMB ASIC SPI readback mismatch--problems communicating with ASICs")
  ExceptionClassGenerator(FEMB_ADC_SYNC_ERROR,"FEMB can't sync FIFOs from ADC ASICs")
}

#endif
