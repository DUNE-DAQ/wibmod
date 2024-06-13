/**
 * @file WIBConfigurator.cpp WIBConfigurator class implementation
 *
 * Based on DataGenerator by Kurt Biery
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "WIBConfigurator.hpp"

#include "appmodel/NetworkConnectionDescriptor.hpp"
#include "appmodel/NetworkConnectionRule.hpp"
#include "appmodel/WIBConfigurator.hpp"
#include "appmodel/WIBConf.hpp"
#include "appmodel/WIBSettings.hpp"
#include "appmodel/WIBPulserSettings.hpp"
#include "appmodel/ColdADCSettings.hpp"
#include "appmodel/FEMBSettings.hpp"

#include "wibmod/Issues.hpp"

#include "logging/Logging.hpp"

#include <string>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "WIBConfigurator"             // NOLINT

namespace dunedaq {
namespace wibmod {

WIBConfigurator::WIBConfigurator(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &WIBConfigurator::do_conf);
  //register_command("settings", &WIBConfigurator::do_settings);
  register_command("start", &WIBConfigurator::do_start);
  register_command("stop", &WIBConfigurator::do_stop);
  register_command("scrap", &WIBConfigurator::do_scrap);
}

void
WIBConfigurator::init(std::shared_ptr<appfwk::ModuleConfiguration> mcfg)
{
  auto dal = mcfg->module<appmodel::WIBConfigurator>(get_name());
  m_wib_conf = dal->get_conf(); //mcfg->module<appmodel::WIBConf>(get_name());
  if (!m_wib_conf) {
    throw appfwk::CommandFailed(ERS_HERE, "init", get_name(), "Unable to retrieve configuration object");
  }
  m_wib_settings = m_wib_conf->get_settings();
}

const appmodel::FEMBSettings* 
WIBConfigurator::femb_conf_i(size_t i)
{
  switch(i) {
    case 0:
      return m_wib_settings->get_femb0();
    case 1:
      return m_wib_settings->get_femb1();
    case 2:
      return m_wib_settings->get_femb2();
    case 3:
      return m_wib_settings->get_femb3();
    default:
      throw UnreachableError(ERS_HERE, get_name());
  }
}

void
WIBConfigurator::populate_femb_conf(wib::ConfigureWIB::ConfigureFEMB *femb_conf, const appmodel::FEMBSettings* conf)
{
  femb_conf->set_enabled(conf->get_enabled());

  femb_conf->set_test_cap(conf->get_test_cap() != 0);
  femb_conf->set_gain(conf->get_gain());
  femb_conf->set_peak_time(conf->get_peak_time());
  femb_conf->set_baseline(conf->get_baseline());
  femb_conf->set_pulse_dac(conf->get_pulse_dac());
  femb_conf->set_gain_match(conf->get_gain_match());

  femb_conf->set_leak(conf->get_leak());
  femb_conf->set_leak_10x(conf->get_leak_10x() != 0);
  femb_conf->set_ac_couple(conf->get_ac_couple());
  femb_conf->set_buffer(conf->get_buffering());

  femb_conf->set_strobe_skip(conf->get_strobe_skip());
  femb_conf->set_strobe_delay(conf->get_strobe_delay());
  femb_conf->set_strobe_length(conf->get_strobe_length());
  
  for (int i = 0; i < conf->get_line_driver().size(); i++) {
    if (i >= 2) {      
      TLOG() <<  "Warning: tried to pass more than 2 line driver values to FEMB configuration";
      break;
    }
    femb_conf->add_line_driver(conf->get_line_driver().at(i));
  }

  for (int i = 0; i < conf->get_pulse_channels().size(); i++) {
    if (i > 15) {
      TLOG() <<  "Warning: tried to pass more than 16 pulse_channel values to FEMB configuration";
      break;
    }
    femb_conf->add_pulse_channels(conf->get_pulse_channels().at(i));
  }
}

void 
WIBConfigurator::do_conf(const data_t& /*conf_as_json*/)
{
  TLOG() << "WIBConfigurator " << get_name() << " is " << m_wib_conf->get_wib_addr();

  wib = std::unique_ptr<WIBCommon>(new WIBCommon(m_wib_conf->get_wib_addr()));

  TLOG() << get_name() << " successfully initialized";
  
  check_timing();

  do_settings();

  check_timing();
}

void
WIBConfigurator::check_timing()
{

  TLOG_DEBUG(0) << get_name() << " Checking timing status";
  wib::GetTimingStatus req;
  wib::GetTimingStatus::TimingStatus rep;
  wib->send_command(req,rep);
  
  int endpoint_status = rep.ept_status() & 0xf;
  if (endpoint_status == 0x8)
  {
    TLOG_DEBUG(0) << get_name() << " timing status correct as " << endpoint_status;
    return;
  } 
  
  TLOG_DEBUG(0) << get_name() << " timing status incorrect as " << endpoint_status; 

  wib::ResetTiming req2;
  wib::GetTimingStatus::TimingStatus rep2;
  wib->send_command(req2,rep2);

  endpoint_status = rep2.ept_status() & 0xf;
  if (endpoint_status == 0x8)
  {
    TLOG_DEBUG(0) << get_name() << " timing status correct as " << endpoint_status;
    return;
  } 
  else
  {
    TLOG_DEBUG(0) << get_name() << " timing status incorrect as " << endpoint_status; 
    throw ConfigurationFailed(ERS_HERE, get_name(), std::to_string(endpoint_status));
  }

}
void
WIBConfigurator::do_settings()
{
  TLOG() << "Building WIB config for " << get_name();
 
  wib::ConfigureWIB req;
  req.set_cold(m_wib_settings->get_cold());
  req.set_pulser(m_wib_settings->get_pulser());
  req.set_adc_test_pattern(m_wib_settings->get_adc_test_pattern());
  req.set_detector_type(m_wib_settings->get_detector_type());

  wib::ConfigureWIB::ConfigureCOLDADC* coldadc_conf = new wib::ConfigureWIB::ConfigureCOLDADC();
  auto coldadc_settings = m_wib_settings->get_coldadc_settings();
  coldadc_conf->set_reg_0(coldadc_settings->get_reg_0());
  coldadc_conf->set_reg_4(coldadc_settings->get_reg_4());
  coldadc_conf->set_reg_24(coldadc_settings->get_reg_24());
  coldadc_conf->set_reg_25(coldadc_settings->get_reg_25());
  coldadc_conf->set_reg_26(coldadc_settings->get_reg_26());
  coldadc_conf->set_reg_27(coldadc_settings->get_reg_27());
  coldadc_conf->set_reg_29(coldadc_settings->get_reg_29());
  coldadc_conf->set_reg_30(coldadc_settings->get_reg_30());
  req.set_allocated_adc_conf(coldadc_conf);
  
  wib::ConfigureWIB::ConfigureWIBPulser* wib_pulser_conf = new wib::ConfigureWIB::ConfigureWIBPulser();
  auto wib_pulser = m_wib_settings->get_wib_pulser();
  wib_pulser_conf->add_femb_en(wib_pulser->get_enabled_0());
  wib_pulser_conf->add_femb_en(wib_pulser->get_enabled_1());
  wib_pulser_conf->add_femb_en(wib_pulser->get_enabled_2());
  wib_pulser_conf->add_femb_en(wib_pulser->get_enabled_3());
  wib_pulser_conf->set_pulse_dac(wib_pulser->get_pulse_dac());
  wib_pulser_conf->set_pulse_period(wib_pulser->get_pulse_period());
  wib_pulser_conf->set_pulse_phase(wib_pulser->get_pulse_phase());
  wib_pulser_conf->set_pulse_duration(wib_pulser->get_pulse_duration());
  req.set_allocated_wib_pulser(wib_pulser_conf);

  for(size_t iFEMB = 0; iFEMB < 4; iFEMB++)
  {
    TLOG() << "Building FEMB " << iFEMB << " config for " << get_name();
    wib::ConfigureWIB::ConfigureFEMB *femb_conf = req.add_fembs();
    populate_femb_conf(femb_conf, femb_conf_i(iFEMB));
  }

  TLOG() << "Sending WIB configuration to " << get_name();
  wib::Status rep;
  wib->send_command(req,rep);
  
  if (rep.success())
  {
    TLOG() << get_name() << " successfully configured";
  }
  else
  {
    TLOG() << get_name() << " failed to configure";
    throw ConfigurationFailed(ERS_HERE, get_name(), rep.extra());
  }
}

void
WIBConfigurator::do_start(const data_t&)
{
  TLOG_DEBUG(0) << get_name() << " successfully started";
}

void
WIBConfigurator::do_stop(const data_t&)
{
  TLOG_DEBUG(0) << get_name() << " successfully stopped";
}

void
WIBConfigurator::do_scrap(const data_t&)
{
  wib = NULL;
  TLOG_DEBUG(0) << get_name() << " successfully scrapped";
}


} // namespace wibmod
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::wibmod::WIBConfigurator)
