/**
 * @file WIBConfigurator.hpp
 *
 * WIBConfigurator is a simple DAQModule implementation that provides a
 * configuration and monitoring interface to the WIB
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
#include "wibmod/wibconfigurator/Nljs.hpp"
#include "wib.pb.h"

#include <appfwk/DAQModule.hpp>
#include <appfwk/ThreadHelper.hpp>

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

  void init(const data_t&) override;

private:
  std::unique_ptr<WIBCommon> wib;

  // Commands
  void do_conf(const data_t&);
  void do_start(const data_t&);
  void do_stop(const data_t&);
  
  // Helpers
  void populate_femb_conf(wib::ConfigureWIB::ConfigureFEMB *femb_conf, const wibconfigurator::FEMBConf &conf);
  const wibconfigurator::FEMBConf& femb_conf_i(const wibconfigurator::WIBConf &conf, size_t i);

};

} // namespace wibmod


ERS_DECLARE_ISSUE_BASE(wibmod,
                       UnreachableError,
                       appfwk::GeneralDAQModuleIssue,
                       "An unreachable part of the code has been reached.",
                       ((std::string)name),
                       ERS_EMPTY)
                       
} // namespace dunedaq

#endif // WIBMOD_PLUGINS_WIBCONFIGURATOR_HPP_
