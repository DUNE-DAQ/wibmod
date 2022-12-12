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
  register_command("settings", &WIBConfigurator::do_settings);
  register_command("start", &WIBConfigurator::do_start);
  register_command("stop", &WIBConfigurator::do_stop);
  register_command("scrap", &WIBConfigurator::do_scrap);
}

void
WIBConfigurator::init(const data_t&)
{
}

const wibconfigurator::FEMBSettings & 
WIBConfigurator::femb_conf_i(const wibconfigurator::WIBSettings &conf, size_t i)
{
  switch(i) {
    case 0:
      return conf.femb0;
    case 1:
      return conf.femb1;
    case 2:
      return conf.femb2;
    case 3:
      return conf.femb3;
    default:
      throw UnreachableError(ERS_HERE, get_name());
  }
}

void
WIBConfigurator::populate_femb_conf(wib::ConfigureWIB::ConfigureFEMB *femb_conf, const wibconfigurator::FEMBSettings &conf)
{
  femb_conf->set_enabled(conf.enabled);

  femb_conf->set_test_cap(conf.test_cap != 0);
  femb_conf->set_gain(conf.gain);
  femb_conf->set_peak_time(conf.peak_time);
  femb_conf->set_baseline(conf.baseline);
  femb_conf->set_pulse_dac(conf.pulse_dac);

  femb_conf->set_leak(conf.leak);
  femb_conf->set_leak_10x(conf.leak_10x != 0);
  femb_conf->set_ac_couple(conf.ac_couple);
  femb_conf->set_buffer(conf.buffering);

  femb_conf->set_strobe_skip(conf.strobe_skip);
  femb_conf->set_strobe_delay(conf.strobe_delay);
  femb_conf->set_strobe_length(conf.strobe_length);
}

void 
WIBConfigurator::do_conf(const data_t& payload)
{

  const wibconfigurator::WIBConf &conf = payload.get<wibconfigurator::WIBConf>();

  TLOG_DEBUG(0) << "WIBConfigurator " << get_name() << " is " << conf.wib_addr;

  wib = std::unique_ptr<WIBCommon>(new WIBCommon(conf.wib_addr));

  TLOG_DEBUG(0) << get_name() << " successfully initialized";
  
  do_settings(conf.settings);

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
  wib::Poke* req2;
  wib::Status rep2;
  req2->set_addr(0xA00C000C); //See WIB firmware document, this is the register for timing edge select (0xA00C000C);
  req2->set_value(0x1);

  wib->send_command(*req2,rep2);
  if(!rep2.success())
  {
    TLOG_DEBUG(0) << get_name() << " failed to write timing edge switch";
    throw ConfigurationFailed(ERS_HERE, get_name(), rep2.extra());
  }

  wib::ResetTiming req3;
  wib::GetTimingStatus::TimingStatus rep3;
  wib->send_command(req3,rep3);

  //Because I've seen the status change after this initial reset
  TLOG_DEBUG(0) << get_name() << " Checking timing status";
  wib::GetTimingStatus req4;
  wib::GetTimingStatus::TimingStatus rep4;  wib->send_command(req4,rep4);

  endpoint_status = rep.ept_status() & 0xf;
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
WIBConfigurator::do_settings(const data_t& payload)
{

  TLOG() << "Building WIB config for " << get_name();
  const wibconfigurator::WIBSettings &conf = payload.get<wibconfigurator::WIBSettings>();
  
  wib::ConfigureWIB req;
  req.set_cold(conf.cold);
  req.set_pulser(conf.pulser);
  req.set_adc_test_pattern(conf.adc_test_pattern);

  for(size_t iFEMB = 0; iFEMB < 4; iFEMB++)
  {
    TLOG() << "Building FEMB " << iFEMB << " config for " << get_name();
    wib::ConfigureWIB::ConfigureFEMB *femb_conf = req.add_fembs();
    populate_femb_conf(femb_conf,femb_conf_i(conf,iFEMB));
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
