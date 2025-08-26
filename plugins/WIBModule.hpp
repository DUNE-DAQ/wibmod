/**
 * @file WIBModule.hpp
 *
 * WIBModule is a simple DAQModule implementation that provides a
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

#include "appmodel/WIBModule.hpp"
#include "appmodel/WIBSettings.hpp"

#include <appfwk/DAQModule.hpp>
#include <utilities/WorkerThread.hpp>

#include <string>
#include <memory>

namespace dunedaq {
namespace wibmod {

/**
 * @brief WIBModule is a simple DAQModule implementation that provides a
 * configuration and monitoring interface to the WIB
 */
class WIBModule : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief WIBModule Constructor
   * @param name Instance name for this WIBModule instance
   */
  explicit WIBModule(const std::string& name);

  WIBModule(const WIBModule&) = delete;            ///< WIBModule is not copy-constructible
  WIBModule& operator=(const WIBModule&) = delete; ///< WIBModule is not copy-assignable
  WIBModule(WIBModule&&) = delete;                 ///< WIBModule is not move-constructible
  WIBModule& operator=(WIBModule&&) = delete;      ///< WIBModule is not move-assignable

  void init(std::shared_ptr<appfwk::ConfigurationManager> mcfg) override;

private:
  std::unique_ptr<WIBCommon> wib;

  // Configuration
  const appmodel::WIBModule* m_wib_conf;
  const appmodel::WIBSettings* m_wib_settings;

  // Commands
  void do_conf(const CommandData_t&);
  void do_settings();
  void check_timing();
  void do_start(const CommandData_t&);
  void do_stop(const CommandData_t&);
  void do_scrap(const CommandData_t&);
  
  // Helpers
  void populate_femb_conf(wib::ConfigureWIB::ConfigureFEMB *femb_conf, const appmodel::FEMBSettings* conf);
  const appmodel::FEMBSettings* femb_conf_i(size_t i);
  bool femb_enabled_i(size_t i);

};

} // namespace wibmod
                       
} // namespace dunedaq

#endif // WIBMOD_PLUGINS_WIBCONFIGURATOR_HPP_
