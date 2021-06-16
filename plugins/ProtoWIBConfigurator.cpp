/**
 * @file ProtoWIBConfigurator.cpp ProtoWIBConfigurator class implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "ProtoWIBConfigurator.hpp"

#include "wibmod/WIB1/WIBException.hh"
#include "wibmod/WIB1/BNL_UDP_Exception.hh"
#include "wibmod/Issues.hpp"

#include "logging/Logging.hpp"

#include <string>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "ProtoWIBConfigurator"        // NOLINT

namespace dunedaq {
namespace wibmod {

ProtoWIBConfigurator::ProtoWIBConfigurator(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &ProtoWIBConfigurator::do_conf);
  register_command("settings", &ProtoWIBConfigurator::do_settings);
  register_command("start", &ProtoWIBConfigurator::do_start);
  register_command("stop", &ProtoWIBConfigurator::do_stop);
  register_command("scrap", &ProtoWIBConfigurator::do_scrap);
}

void
ProtoWIBConfigurator::init(const data_t&)
{
}

void
ProtoWIBConfigurator::do_conf(const data_t& payload)
{
  const protowibconfigurator::WIBConf &conf = payload.get<protowibconfigurator::WIBConf>();

  TLOG_DEBUG(0) << "ProtoWIBConfigurator " << get_name() << " is " << conf.wib_addr;
  
  wib = std::make_unique<WIB>( conf.wib_addr, conf.wib_table, conf.femb_table );
  
  TLOG_DEBUG(0) << get_name() << " successfully initialized";
}
  
void
ProtoWIBConfigurator::do_settings(const data_t& payload)
{
  const protowibconfigurator::WIBSettings &conf = payload.get<protowibconfigurator::WIBSettings>();
  
  if(conf.partition_number > 15)
  {
    throw InvalidPartitionNumber(ERS_HERE, get_name(), conf.partition_number);
  }

  // Set DIM do-not-disturb
  wib->Write("SYSTEM.SLOW_CONTROL_DND", 1);
  // makes sure monitoring notices DND before configuring
  std::this_thread::sleep_for(std::chrono::seconds(conf.dnd_wait_time));

  // If these are true, will continue on error, if false, will raise an exception
  wib->SetContinueOnFEMBRegReadError(conf.continue_on_femb_reg_read_error);
  wib->SetContinueOnFEMBSPIError(conf.continue_on_femb_spi_error);
  wib->SetContinueOnFEMBSyncError(conf.continue_on_femb_sync_error);
  wib->SetContinueIfListOfFEMBClockPhasesDontSync(conf.continue_if_close_phases_dont_sync);
  
  // Check if WIB firmware is for RCE or FELIX DAQ
  TLOG_DEBUG(0) << "N DAQ Links: "  << wib->Read("SYSTEM.DAQ_LINK_COUNT");
  TLOG_DEBUG(0) << "N FEMB Ports: "  << wib->Read("SYSTEM.FEMB_COUNT");
  WIB::WIB_DAQ_t daqMode = wib->GetDAQMode();
  
  std::string expected_daq_mode = conf.expected_daq_mode;
  
  if (daqMode == WIB::RCE)
  {
    TLOG_DEBUG(0) << "WIB Firmware setup for RCE DAQ Mode";
    if(expected_daq_mode != "RCE" &&
       expected_daq_mode != "rce" && 
       expected_daq_mode != "ANY" && 
       expected_daq_mode != "any"
      )
    {
      throw WrongFirmwareMode(ERS_HERE, get_name(), "RCE", expected_daq_mode);
    }
  }
  else if (daqMode == WIB::FELIX)
  {
    TLOG_DEBUG(0) << "WIB Firmware setup for FELIX DAQ Mode";
    if(expected_daq_mode != "FELIX" && 
       expected_daq_mode != "felix" &&
       expected_daq_mode != "ANY" &&
       expected_daq_mode != "any"
      )
    {
      throw WrongFirmwareMode(ERS_HERE, get_name(), "FELIX", expected_daq_mode);
    }
  }
  else if (daqMode == WIB::UNKNOWN)
  {
    throw UnknownFirmwareMode(ERS_HERE, get_name());
  }
  else
  {
    throw UnknownFirmwareMode(ERS_HERE, get_name());
  }

  // Check and print firmware version
  uint32_t expected_wib_fw_version = conf.expected_wib_fw_version;
  uint32_t wib_fw_version = wib->Read("SYSTEM.FW_VERSION");
  
  TLOG_DEBUG(0) << "WIB Firmware Version: 0x" 
        << std::hex << std::setw(8) << std::setfill('0')
        <<  wib_fw_version
        << " Synthesized: " 
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.CENTURY")
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.YEAR") << "-"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.MONTH") << "-"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_DATE.DAY") << " "
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_TIME.HOUR") << ":"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_TIME.MINUTE") << ":"
        << std::hex << std::setw(2) << std::setfill('0')
        << wib->Read("SYSTEM.SYNTH_TIME.SECOND");
  
  if (expected_wib_fw_version != wib_fw_version)
  {
    throw IncorrectFirmwareVersion(ERS_HERE, get_name(), "WIB", wib_fw_version, expected_wib_fw_version);
  }

  // Reset and setup clock
  if (conf.force_full_reset)
  {
    TLOG_DEBUG(0) << "Running Full Reset on the WIB";
    wib->ResetWIBAndCfgDTS(conf.local_clock, conf.partition_number, conf.dts_source);
  }
  else
  {
    TLOG_DEBUG(0) << "Running Checked Reset on the WIB";
    wib->CheckedResetWIBAndCfgDTS(conf.local_clock, conf.partition_number, conf.dts_source);    
    TLOG_DEBUG(0) << "Finished Checked Reset on the WIB";
    if (daqMode == WIB::FELIX)
    {                                                  
      TLOG_DEBUG(0) << "SI5342 Status: " << wib->Read("DAQ.SI5342.ENABLE");
    }                                               
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Check DAQ link copy mode
  if (wib->Read("FEMB_REPLACE_FEMB_3_N_5_WITH_1_N_2"))
  {
    throw BadDuplicationMode(ERS_HERE, get_name());
  }
  
  // Configure WIB fake data enable and mode
  TLOG_DEBUG(0) << "Configuring WIB Fake Data";
  TLOG_DEBUG(0) << "Is Fake:"
                << " FEMB1: " << femb_conf_i(conf, 0).enable_wib_fake_data
                << " FEMB2: " << femb_conf_i(conf, 1).enable_wib_fake_data
                << " FEMB3: " << femb_conf_i(conf, 2).enable_wib_fake_data
                << " FEMB4: " << femb_conf_i(conf, 3).enable_wib_fake_data;
  wib->ConfigWIBFakeData(femb_conf_i(conf, 0).enable_wib_fake_data, 
                         femb_conf_i(conf, 1).enable_wib_fake_data, 
                         femb_conf_i(conf, 2).enable_wib_fake_data, 
                         femb_conf_i(conf, 3).enable_wib_fake_data, 
                         conf.use_wib_fake_data_counter);
  
  // Configure FEMBs
  for (size_t iFEMB = 1; iFEMB <= 4; iFEMB++)
  {
    const protowibconfigurator::FEMBSettings &FEMB_conf = femb_conf_i(conf, iFEMB-1);
    if (FEMB_conf.enabled)
    {

      if (FEMB_conf.enable_femb_fake_data)
      {
        TLOG_DEBUG(0) << "Setting up FEMB"<<iFEMB<<" for fake data";
        setup_femb_fake_data(iFEMB, FEMB_conf, conf.continue_on_femb_reg_read_error);
      }
      else
      {
        TLOG_DEBUG(0) << "Setting up FEMB"<<iFEMB;
        setup_femb(iFEMB, FEMB_conf, conf.continue_on_femb_reg_read_error);
      }
    }
    else
    {
      TLOG_DEBUG(0) << "FEMB"<<iFEMB<<" not enabled";
    }
  }

  if (!((daqMode == WIB::FELIX) && conf.start_felix_links_at_run_start))
  {
    // don't enable links yet if FELIX and start_links_FELIX, do it in start
    TLOG_DEBUG(0) << "Enabling DAQ links";
    wib->StartStreamToDAQ();
  }
  
  // Un-set DIM do-not-disturb
  wib->Write("SYSTEM.SLOW_CONTROL_DND", 0);

  TLOG_DEBUG(0) << "Configured WIB";
}

void ProtoWIBConfigurator::setup_femb_fake_data(size_t iFEMB, const protowibconfigurator::FEMBSettings& FEMB_conf, bool continue_on_reg_read_error) {
  wib->FEMBPower(iFEMB, 1);
  sleep(5);

  if (wib->ReadFEMB(iFEMB, "VERSION_ID") == wib->ReadFEMB(iFEMB, "SYS_RESET")) 
  { // can't read register if equal
    if (continue_on_reg_read_error)
    {
      ers::warning(CannotReadFromFEMB(ERS_HERE, get_name(), "Can't read registers from FEMB"+std::to_string(iFEMB)+"; power down and continue"));
      wib->FEMBPower(iFEMB, 0);
      return;
    }
    else
    {
      wib->FEMBPower(iFEMB, 0);
      throw CannotReadFromFEMB(ERS_HERE, get_name(), "Can't read registers from FEMB"+std::to_string(iFEMB));
    }
  }

  auto expected_femb_fw_version = FEMB_conf.expected_femb_fw_version;
  uint32_t femb_fw_version = wib->ReadFEMB(iFEMB, "VERSION_ID");
  if (expected_femb_fw_version != femb_fw_version)
  {
    throw IncorrectFirmwareVersion(ERS_HERE, get_name(), "FEMB"+std::to_string(iFEMB), femb_fw_version, expected_femb_fw_version);
  }

  uint8_t fake_mode = 0;
  uint16_t fake_word = 0;
  uint8_t femb_number = iFEMB;
  std::vector<uint32_t> fake_waveform(255);
  
  auto fakeDataSelect = FEMB_conf.fake_data_select;
  if (fakeDataSelect == "fake_word")
  {
    fake_mode = 1;
    fake_word = FEMB_conf.fake_word;
  }
  else if (fakeDataSelect == "fake_waveform")
  {
    fake_mode = 2;
    if (fake_waveform.size() != 255)
    {
      std::stringstream excpt;
      excpt << "setup_femb_fake_data: FEMB "
          << iFEMB
          << " fake_waveform is not implemented!";
      throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
    }
  }
  else if (fakeDataSelect == "femb_channel_id")
  {
    fake_mode = 3;
  }
  else if (fakeDataSelect == "counter_channel_id")
  {
    fake_mode = 4;
  }
  else
  {
    std::stringstream excpt;
    excpt << "FEMB" << iFEMB << " fake_data_select is \""
        << fakeDataSelect
        <<"\" but expect "
        <<" fake_word, fake_waveform, femb_channel_id, or counter_channel_id";
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }

  wib->ConfigFEMBFakeData(iFEMB, fake_mode, fake_word, femb_number, fake_waveform);
}

void ProtoWIBConfigurator::setup_femb(size_t iFEMB, const protowibconfigurator::FEMBSettings& FEMB_conf, bool continue_on_reg_read_error){
  if (FEMB_conf.gain > 3)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " gain shouldn't be larger than 3 is: "
        << FEMB_conf.gain;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.shape > 3)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " shape shouldn't be larger than 3 is: "
        << FEMB_conf.shape;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.baseline_high > 2)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " baseline_high should be 0 or 1 or 2 is: "
        << FEMB_conf.baseline_high;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.leak_high > 1)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " leak_high should be 0 or 1 is: "
        << FEMB_conf.leak_high;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.leak_10x > 1)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " leak_10x should be 0 or 1 is: "
        << FEMB_conf.leak_10x;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.ac_couple > 1)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " ac_couple should be 0 or 1 is: "
        << FEMB_conf.ac_couple;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.buffer > 1)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " buffer should be 0 or 1 is: "
        << FEMB_conf.buffer;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.ext_clk > 1)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " ext_clk should be 0 or 1 is: "
        << FEMB_conf.ext_clk;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.clk_phases.size() == 0)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " clk_phases size should be > 0 ";
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.pulse_mode > 2)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " pulse_mode should be 0 (off) 1 (FE ASIC internal) or 2 (FPGA external) is: "
        << FEMB_conf.pulse_mode;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if ((FEMB_conf.pulse_mode == 1 && FEMB_conf.pulse_dac > 63) || (FEMB_conf.pulse_mode == 2 && FEMB_conf.pulse_dac > 31))
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " pulse_dac should be 0-31 in pulse_mode 1 or 0-63 in pulse_mode 2."
        << " pulse_mode is " << FEMB_conf.pulse_mode
        << " pulse_dac is " << FEMB_conf.pulse_dac;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.start_frame_mode > 1)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " start_frame_mode should be 0 or 1 is: "
        << FEMB_conf.start_frame_mode;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }
  if (FEMB_conf.start_frame_swap > 1)
  {
    std::stringstream excpt;
    excpt << "setup_femb: FEMB "
        << iFEMB
        << " start_frame_swap should be 0 or 1 is: "
        << FEMB_conf.start_frame_swap;
    throw InvalidFEMBSetting(ERS_HERE, get_name(), excpt.str());
  }

  ///////////////////////////////////////

  wib->FEMBPower(iFEMB, 1);
  sleep(5);

  if (wib->ReadFEMB(iFEMB, "VERSION_ID") == wib->ReadFEMB(iFEMB, "SYS_RESET")) 
  { // can't read register if equal
    if (continue_on_reg_read_error)
    {
      ers::warning(CannotReadFromFEMB(ERS_HERE, get_name(), "Can't read registers from FEMB"+std::to_string(iFEMB)+"; power down and continue"));
      wib->FEMBPower(iFEMB, 0);
      return;
    }
    else
    {
      wib->FEMBPower(iFEMB, 0);
      throw CannotReadFromFEMB(ERS_HERE, get_name(), "Can't read registers from FEMB"+std::to_string(iFEMB));
    }
  }

  uint32_t femb_fw_version = wib->ReadFEMB(iFEMB, "VERSION_ID");
  if (FEMB_conf.expected_femb_fw_version != femb_fw_version)
  {
    throw IncorrectFirmwareVersion(ERS_HERE, get_name(), "FEMB"+std::to_string(iFEMB), femb_fw_version, FEMB_conf.expected_femb_fw_version);
  }

  std::vector<uint32_t> fe_config = {FEMB_conf.gain, 
                                     FEMB_conf.shape, 
                                     FEMB_conf.baseline_high, 
                                     FEMB_conf.leak_high, 
                                     FEMB_conf.leak_10x, 
                                     FEMB_conf.ac_couple, 
                                     FEMB_conf.buffer, 
                                     FEMB_conf.ext_clk};

  wib->ConfigFEMB(iFEMB, fe_config, FEMB_conf.clk_phases, FEMB_conf.pulse_mode, 
                  FEMB_conf.pulse_dac, FEMB_conf.start_frame_mode, 
                  FEMB_conf.start_frame_swap);

}

void
ProtoWIBConfigurator::do_start(const data_t&)
{
  if (!wib) 
  {
    throw CommandFailed(ERS_HERE, get_name(), "WIB object pointer NULL");
  }
  if ((wib->GetDAQMode() == WIB::FELIX) && start_links_FELIX_run_start)
  {
    TLOG_DEBUG(0) << "Enabling DAQ links";

    unsigned start_run_tries = 5;
    bool success = false;
    for (unsigned iTry=1; iTry <= start_run_tries; iTry++)
    {
      try
      {
        wib->Write("SYSTEM.SLOW_CONTROL_DND", 1);
        wib->StartStreamToDAQ();
        success = true;
        break;
      }
      catch (const BUException::BAD_REPLY &exc)
      {
        ers::warning(WIBCommunicationError(ERS_HERE, get_name(), exc.what()));
      }
      catch (const BUException::exBase &exc)
      {
        // Try to un-set DIM do-not-disturb no matter what happened
        try
        {
          if (wib) wib->Write("SYSTEM.SLOW_CONTROL_DND", 0);
        }
        catch (const BUException::exBase &exc)
        {
          // best effort, don't care if it doesn't succeed
        }

        throw UnhandledBUException(ERS_HERE, get_name(), exc.what(), exc.Description());
      }
      TLOG_DEBUG(0) << "Run start try  " << iTry << " failed. Trying again...";
    } // for iRetry

    // Try to un-set DIM do-not-disturb no matter what happened
    try
    {
      if (wib) wib->Write("SYSTEM.SLOW_CONTROL_DND", 0);
    }
    catch (const BUException::exBase & exc)
    {
      // best effort, don't care if it doesn't succeed
    }

    if (!success)
    {
      throw CommandFailed(ERS_HERE, get_name(), "Failed to start run after " + 
                          std::to_string(start_run_tries) + " tries");
    }
  } // if felix
}

void
ProtoWIBConfigurator::do_stop(const data_t&)
{
  if (!wib) 
  {
    throw CommandFailed(ERS_HERE, get_name(), "WIB object pointer NULL");
  }
  if (wib->GetDAQMode() == WIB::FELIX && stop_links_FELIX_run_stop)
  {
    TLOG_DEBUG(0) << "Disabling DAQ links";

    unsigned stop_run_tries = 5;
    bool success = false;
    for (unsigned iTry=1; iTry <= stop_run_tries; iTry++)
    {
      try
      {
        wib->Write("SYSTEM.SLOW_CONTROL_DND", 1);
        wib->Write("DAQ_LINK_1.CONTROL.ENABLE", 0);
        wib->Write("DAQ_LINK_2.CONTROL.ENABLE", 0);
        //wib->Write("DAQ_LINK_1.CONTROL.ENABLE_CDA_STREAM", 0);
        //wib->Write("DAQ_LINK_2.CONTROL.ENABLE_CDA_STREAM", 0);
        success = true;
        break;
      }
      catch (const BUException::BAD_REPLY &exc)
      {
        ers::warning(WIBCommunicationError(ERS_HERE, get_name(), exc.what()));
      }
      catch (const BUException::exBase &exc)
      {
        // Try to un-set DIM do-not-disturb no matter what happened
        try
        {
          if (wib) wib->Write("SYSTEM.SLOW_CONTROL_DND", 0);
        }
        catch (const BUException::exBase &exc)
        {
          // best effort, don't care if it doesn't succeed
        }

        throw UnhandledBUException(ERS_HERE, get_name(), exc.what(), exc.Description());
      }
      TLOG_DEBUG(0) << "Run stop try  " << iTry << " failed. Trying again...";
    } // for iRetry

    // Try to un-set DIM do-not-disturb no matter what happened
    try
    {
      if (wib) wib->Write("SYSTEM.SLOW_CONTROL_DND", 0);
    }
    catch (const BUException::exBase &exc)
    {
      // best effort, don't care if it doesn't succeed
    }

    if (!success)
    {
      throw CommandFailed(ERS_HERE, get_name(), "Failed to stop run after " + 
                          std::to_string(stop_run_tries) + " tries");
    }
  } // if felix
}

void
ProtoWIBConfigurator::do_scrap(const data_t&)
{
  wib = NULL;
  TLOG_DEBUG(0) << get_name() << " successfully scrapped";
}



} // namespace wibmod
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::wibmod::ProtoWIBConfigurator)
