/**
 * @file WIBConfigurator.hpp
 *
 * WIBConfigurator is a simple DAQModule implementation that provides a
 * configuration and monitoring interface to the WIB2
 *
 * Based on DataGenerator by Kurt Biery
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef WIBMOD_PLUGINS_WIBCONFIGURATOR_HPP_
#define WIBMOD_PLUGINS_WIBCONFIGURATOR_HPP_

#include "wibmod/WIBCommon.hpp"
//#include "wibmod/wibconfigurator/Nljs.hpp"
#include "wib.pb.h"

#include "appmodel/WIBConfigurator.hpp"
#include "appmodel/WIBSettings.hpp"

#include <appfwk/DAQModule.hpp>
#include <utilities/WorkerThread.hpp>

#include <string>
#include <memory>

namespace dunedaq {
namespace wibmod {

/**
 * @brief WIBConfigurator is a simple DAQModule implementation that provides a
 * configuration and monitoring interface to the WIB
 */
class WIBConfigurator : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief WIBConfigurator Constructor
   * @param name Instance name for this WIBConfigurator instance
   */
  explicit WIBConfigurator(const std::string& name);

  WIBConfigurator(const WIBConfigurator&) = delete;            ///< WIBConfigurator is not copy-constructible
  WIBConfigurator& operator=(const WIBConfigurator&) = delete; ///< WIBConfigurator is not copy-assignable
  WIBConfigurator(WIBConfigurator&&) = delete;                 ///< WIBConfigurator is not move-constructible
  WIBConfigurator& operator=(WIBConfigurator&&) = delete;      ///< WIBConfigurator is not move-assignable

  void init(std::shared_ptr<appfwk::ModuleConfiguration> mcfg) override;

private:
  std::unique_ptr<WIBCommon> wib;

  // Configuration
  const appmodel::WIBConfigurator* m_wib_conf;
  const appmodel::WIBSettings* m_wib_settings;

  // Commands
  void do_conf(const data_t&);
  void do_settings();
  void check_timing();
  void do_start(const data_t&);
  void do_stop(const data_t&);
  void do_scrap(const data_t&);
  
  // Helpers
  void populate_femb_conf(wib::ConfigureWIB::ConfigureFEMB *femb_conf, const appmodel::FEMBSettings* conf);
  const appmodel::FEMBSettings* femb_conf_i(size_t i);

};

} // namespace wibmod
                       
} // namespace dunedaq

#endif // WIBMOD_PLUGINS_WIBCONFIGURATOR_HPP_
