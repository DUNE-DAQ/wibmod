/**
 * @file WIBConfigurator.cpp WIBConfigurator class implementation
 *
 * Based on DataGenerator by Kurt Biery
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "logging/Logging.hpp"

#include "WIBConfigurator.hpp"

#include <string>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "WIBConfigurator"             // NOLINT
#define TLVL_ENTER_EXIT_METHODS TLVL_DEBUG + 5   // NOLINT
#define TLVL_WORK_STEPS TLVL_DEBUG + 10          // NOLINT

namespace dunedaq {
namespace wibmod {

WIBConfigurator::WIBConfigurator(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &WIBConfigurator::do_conf);
  register_command("start", &WIBConfigurator::do_start);
  register_command("stop", &WIBConfigurator::do_stop);
}

void
WIBConfigurator::init(const data_t&)
{
}

const wibconfigurator::FEMBConf & 
WIBConfigurator::femb_conf_i(const wibconfigurator::WIBConf &conf, size_t i)
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
WIBConfigurator::populate_femb_conf(wib::ConfigureWIB::ConfigureFEMB *femb_conf, const wibconfigurator::FEMBConf &conf)
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
  femb_conf->set_buffer(conf.buffer);

  femb_conf->set_strobe_skip(conf.strobe_skip);
  femb_conf->set_strobe_delay(conf.strobe_delay);
  femb_conf->set_strobe_length(conf.strobe_length);
}

void
WIBConfigurator::do_conf(const data_t& payload)
{
  const wibconfigurator::WIBConf &conf = payload.get<wibconfigurator::WIBConf>();

  wib = std::unique_ptr<WIBCommon>(new WIBCommon(conf.wib_addr));

  TLOG_DEBUG(0) << "Building WIB config for " << conf.wib_addr;
  wib::ConfigureWIB req;
  req.set_cold(conf.cold);
  req.set_pulser(conf.pulser);
  req.set_adc_test_pattern(conf.adc_test_pattern);

  for(size_t iFEMB = 0; iFEMB < 4; iFEMB++)
  {
    TLOG_DEBUG(0) << "Building FEMB " << iFEMB << " config for " << conf.wib_addr;
    wib::ConfigureWIB::ConfigureFEMB *femb_conf = req.add_fembs();
    populate_femb_conf(femb_conf,femb_conf_i(conf,iFEMB));
  }

  TLOG_DEBUG(0) << "Sending WIB configuration to " << conf.wib_addr;
  wib::Status rep;
  wib->send_command(req,rep);
  
  if (rep.success())
  {
    TLOG_DEBUG(0) << conf.wib_addr << " successfully configured";
  }
  else
  {
    TLOG_DEBUG(0) << conf.wib_addr << " failed to configure";
    //FIXME raise fatal error
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


} // namespace wibmod
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::wibmod::WIBConfigurator)
