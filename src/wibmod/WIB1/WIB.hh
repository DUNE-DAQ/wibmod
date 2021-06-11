#ifndef __WIB_HH__
#define __WIB_HH__

#include <WIB/WIBBase.hh>
#include <stdint.h>

// Keeps artDAQ from complaining about __int128
// not being in C++ standard
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

typedef unsigned __int128 uint128_t;

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

struct data_8b10b_t{
  data_8b10b_t(uint8_t _k, uint8_t _d){k=_k;data=_d;}
  bool k;
  uint8_t data;
};

class WIB: public WIBBase {
 public:
  WIB(std::string const & address, std::string const & WIBAddressTable = "WIB.adt", std::string const & FEMBAddressTable = "FEMB.adt", bool fullStart=true);
  ~WIB();  
  
  //To be used only within the DIM server, if the WIB is initially down
  void FullStart();
  bool started;

  //initialize hardware
  void InitializeWIB();
  void ResetWIB(bool reset_udp=false);
  void InitializeDTS(uint8_t PDTSsource = 0,uint8_t clockSource = 0, uint32_t PDTSAlignment_timeout = 0 /*default infinite*/);
  void EnableDAQLink(uint8_t iDAQLink);
  void EnableDAQLink_Lite(uint8_t iDAQLink,uint8_t enable);
  void StartSyncDTS();
  void ResetWIBAndCfgDTS(uint8_t localClock,uint8_t PDTS_TGRP, uint8_t PDTSsource = 0, uint32_t PDTSAlignment_timeout = 0);
  void CheckedResetWIBAndCfgDTS(uint8_t localClock,uint8_t PDTS_TGRP, uint8_t PDTSsource = 0, uint32_t PDTSAlignment_timeout = 0);
  void StartStreamToDAQ();
  void PDTSInRunningState();

  void EnableFEMBCNC();
  void DisableFEMBCNC();
  void FEMBPower(uint8_t iFEMB,bool turnOn);
  void SourceFEMB(uint64_t iDAQLink,uint64_t real);

  //Event builder control
  void StartEventBuilder(uint8_t mask = 0xF);
  void StopEventBuilder(uint8_t mask = 0xF);

  //QSFP
  void WriteQSFP(uint16_t address,uint32_t value,uint8_t byte_count);
  uint32_t ReadQSFP(uint16_t address,uint8_t byte_count);

  //DTS CDS
  void WriteDTS_CDS(uint16_t address,uint32_t value,uint8_t byte_count = 4,bool ignore_error = false);
  uint32_t ReadDTS_CDS(uint16_t address,uint8_t byte_count = 4);
  float ConfigureDTSCDS(uint8_t source = 0);

  //DTS SI5344
  void     WriteDTS_SI5344(uint16_t address,uint32_t value,uint8_t byte_count = 4);
  uint32_t ReadDTS_SI5344(uint16_t address,uint8_t byte_count = 4);
  void     SetDTS_SI5344Page(uint8_t page);
  uint8_t  GetDTS_SI5344Page();
  uint8_t  GetDTS_SI5344AddressPage(uint16_t address);
  void     LoadConfigDTS_SI5344(std::string const & fileName);
  void     ResetSi5344();
  void     SelectSI5344(uint64_t input, bool enable);
  void     SelectSI5342(uint64_t input, bool enable);

  //DAQ SI5342
  void     WriteDAQ_SI5342(uint16_t address,uint32_t value,uint8_t byte_count = 4);
  uint32_t ReadDAQ_SI5342(uint16_t address,uint8_t byte_count = 4);
  void     SetDAQ_SI5342Page(uint8_t page);
  uint8_t  GetDAQ_SI5342Page();
  uint8_t  GetDAQ_SI5342AddressPage(uint16_t address);
  void     LoadConfigDAQ_SI5342(std::string const & fileName);
  void     ResetSi5342();

  //History debug
  std::vector<uint32_t> CaptureHistory(std::string const & address);
  std::vector<uint128_t> CaptureHistory(std::string const & address,size_t wordCount);

  //Local FLASH
  uint32_t ReadLocalFlash(uint16_t address);
  std::vector<uint32_t> ReadLocalFlash(uint16_t address,size_t n);
  void WriteLocalFlash(uint16_t address,uint32_t data);
  void WriteLocalFlash(uint16_t address,std::vector<uint32_t> const  & data);


  //Flash
  void FlashCheckBusy();
  void ReadFlash(std::string const & fileName,uint8_t update_percentage = 101);
  void WriteFlash(std::vector<uint32_t> data,uint8_t update_percentage = 101);
  void ProgramFlash(std::string const & fileName,uint8_t update_percentage = 101);
  void EraseFlash(bool print_updates =false);
  void CheckFlash(std::vector<uint32_t> data,uint8_t update_percentage = 101);

  //CD link spy buffer
  std::vector<data_8b10b_t> ReadOutCDLinkSpyBuffer();

  //Event builder spy buffer
  std::vector<data_8b10b_t> ReadDAQLinkSpyBuffer(uint8_t iDAQLink, uint8_t trigger_mode = 0);

  //FEMB Configuration
  
  /** \brief Setup FEMB External Clock
   *
   *  Sets up iFEMB (index from 1) external clock parameters
   */
  void SetupFEMBExtClock(uint8_t iFEMB);

  //config_phase from shanshan's scripts
  void WriteFEMBPhase(uint8_t iFEMB, uint16_t clk_phase_data);
  bool TryFEMBPhases(uint8_t iFEMB, std::vector<uint16_t> phases);
  bool HuntFEMBPhase(uint8_t iFEMB, uint16_t clk_phase_data_start);

  /** \brief Setup FEMB in real or pulser data mode
   *
   *  Sets up iFEMB (index from 1)
   *  fe_config: list of options to configure the FE ASICs:
   *          Gain: 0,1,2,3 for 4.7, 7.8, 14, 25 mV/fC, respectively
   *          Shaping Time: 0,1,2,3 for 0.5, 1, 2, 3 us, respectively
   *          High Baseline: 0 for 200 mV, 1 for 900 mV, 2 for 200 mV on collection and 900 mV on induction
   *          High Leakage: 0 for 100 pA, 1 for 500 pA
   *          Leakage x 10: if 1, multiply leakage times 10
   *          AC Coupling : 0 for DC coupling, 1 for AC coupling (between FE and ADC)
   *          Buffer: 0 for disable and bypass, 1 for use (between FE and ADC)
   *          Use External Clock: 0 ADC use internal clock, 1 ADC use FPGA clocking (almost always want 1)
   *  clk_phases: a list of 16 bit values to try for the ADC clock phases.
   *      Tries these values until the sync check bits are all 0, and hunts 
   *        for good values if these all fail.
   *      The most significant byte is ADC_ASIC_CLK_PHASE_SELECT (register 6)
   *        while the least significant byte is ADC_ASIC_CLK_PHASE_SELECT (register 15)
   *  pls_mode: pulser mode select: 0 off, 1 FE ASIC internal pulser, 2 FPGA pulser
   *  pls_dac_val: pulser DAC value (amplitude) 
   *      6-bits in ASIC test pulse mode, 5-bits in FPGA test pulse mode
   *  start_frame_mode_sel: 1 to make data frame start the way BU WIB firmware expects
   *  start_frame_swap: 1 to reverse the start bits
   */
  void ConfigFEMB(uint8_t iFEMB, std::vector<uint32_t> fe_config, std::vector<uint16_t> clk_phases,
                    uint8_t pls_mode=0, uint8_t pls_dac_val=0, uint8_t start_frame_mode_sel=1, uint8_t start_frame_swap=1);
  /** \brief Setup FEMB in fake data mode
   *
   *  Sets up iFEMB (index from 1) in fake data mode
   *  fake_mode: 0 for real data, 1 for fake word, 2 for fake waveform, 3 for channel indicator (FEMB, chip, channel),
   *          4 for channel indicator (counter, chip, channel)
   *  fake_word: 12 bit wrd to use when in fake word mode
   *  femb_number: femb number to use in fake_mode 3
   *  fake_samples: vector of samples to use in fake_mode 2
   */
  void ConfigFEMBFakeData(uint8_t iFEMB, uint8_t fake_mode, uint32_t fake_word, uint8_t femb_number, 
            std::vector<uint32_t> fake_samples, uint8_t start_frame_mode_sel=1, uint8_t start_frame_swap=1);
  void ConfigFEMBMode(uint8_t iFEMB, uint32_t pls_cs, uint32_t dac_sel, uint32_t fpga_dac, uint32_t asic_dac, uint32_t mon_cs);
  /** \brief Setup FEMB ASICs
   *
   *  Sets up iFEMB (index from 1) ASICs
   *
   *  gain: 0,1,2,3 for 4.7, 7.8, 14, 25 mV/fC, respectively
   *  shaping time: 0,1,2,3 for 0.5, 1, 2, 3 us, respectively
   *  highBaseline is 900mV for 1, 200mV for 0, and appropriately for each plane for 2
   *  highLeakage is 500pA for true, 100pA for false
   *  leakagex10 multiplies leakage x10 if true
   *  acCoupling: FE is AC coupled to ADC if true, DC if false
   *  buffer: FE to ADC buffer on if true, off and bypassed if false
   *  useExtClock: ADC uses external (FPGA) clock if true, internal if false
   *  internalDACControl: 0 for disabled, 1 for internal FE ASIC pulser, 2 for external FPGA pulser
   *  internalDACValue: 6 bit value for amplitude to use with internal pulser
   *
   *  returns adc sync status 16 bits, one for each serial link between ADC and FPGA. There are 2 per ADC
   */
  uint16_t SetupFEMBASICs(uint8_t iFEMB, uint8_t gain, uint8_t shape, uint8_t highBaseline, 
                        bool highLeakage, bool leakagex10, bool acCoupling, bool buffer, bool useExtClock, 
                        uint8_t internalDACControl, uint8_t internalDACValue);

  void SetupFPGAPulser(uint8_t iFEMB, uint8_t dac_val);
  void SetupInternalPulser(uint8_t iFEMB);
  uint16_t SetupASICPulserBits(uint8_t iFEMB);

  /** \brief Setup FEMB ASICs
   *
   *  Sets up iFEMB (index from 1) ASICs
   *
   *  registerList is a list of 71 32bit registers to program the FE and ADC ASICs
   *
   *  returns adc sync status 16 bits, one for each serial link between ADC and FPGA. There are 2 per ADC
   */
  uint16_t SetupFEMBASICs(uint8_t iFEMB, std::vector<uint32_t> registerList);

  //Debug
  void    ConfigWIBFakeData(bool enableFakeFEMB1, bool enableFakeFEMB2, 
                                bool enableFakeFEMB3, bool enableFakeFEMB4, 
                                bool counter); // counter==true: counter instead of COLDATA frame, else samples in COLDATA frame
  void    SetFEMBFakeCOLDATAMode(uint8_t iFEMB,uint8_t iCD, bool mode = 0);
  uint8_t GetFEMBFakeCOLDATAMode(uint8_t iFEMB,uint8_t iCD);
  void    SetFEMBStreamSource(uint8_t iFEMB,uint8_t iStream,bool real=true);
  uint8_t GetFEMBStreamSource(uint8_t iFEMB,uint8_t iStream);
//  void    SetFakeCOLDATA(uint8_t mask = 0xFF);
//  uint8_t GetFakeCOLDATA();
  void    SetEventBuilderDebugMode(uint8_t mask = 0xF);
  uint8_t GetEventBuilderDebugMode();

  enum WIB_DAQ_t {UNKNOWN,RCE,FELIX};

  //Helper
  char GetFEMBChar(uint8_t iFEMB);
  char GetDAQLinkChar(uint8_t iDAQLink);
  char GetFEMBCDChar(uint8_t iCD);
  bool CheckDAQLinkInRange(uint8_t iDAQLink);
  bool CheckFEMBInRange(uint8_t iFEMB);
  bool CheckFEMBStreamInRange(uint8_t iStream);
  bool CheckFEMBCDInRange(uint8_t iCD);
  uint8_t GetFEMBCount(){return FEMBCount;}
  WIB_DAQ_t GetDAQMode(){return DAQMode;}

  void SetContinueOnFEMBRegReadError(bool enable);
  void SetContinueOnFEMBSPIError(bool enable);
  void SetContinueOnFEMBSyncError(bool enable);
  void SetContinueIfListOfFEMBClockPhasesDontSync(bool enable); // if true try to hunt for the phase else raise exception

 private:
  WIB(); //disallow the default constructor
  // Prevent copying of WIB objects
  WIB( const WIB& other) ; // prevents construction-copy
  WIB& operator=( const WIB&) ; // prevents copying
  WIB_DAQ_t DAQMode;
  uint8_t FEMBCount;
  uint8_t FEMBStreamCount;
  uint8_t FEMBCDACount;
  uint8_t DAQLinkCount;

  bool ContinueOnFEMBRegReadError;
  bool ContinueOnFEMBSPIError;
  bool ContinueOnFEMBSyncError; // if phase hunt fails keep going else raise exception
  bool ContinueIfListOfFEMBClockPhasesDontSync; // if true try to hunt for the phase else raise exception
};
#endif
