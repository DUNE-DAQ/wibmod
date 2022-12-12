/**
 * @file ProtoWIBConfigurator.hpp
 *
 * ProtoWIBConfigurator is a simple DAQModule implementation that provides a
 * configuration and monitoring interface to the WIB1.
 *
 * Pretty much directly stolen from artDAQ's WIBReader
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef WIBMOD_PLUGINS_PROTOWIBCONFIGURATOR_HPP_
#define WIBMOD_PLUGINS_PROTOWIBCONFIGURATOR_HPP_

#include "wibmod/WIB1/WIB.hh"
#include "wibmod/protowibconfigurator/Nljs.hpp"

#include <appfwk/DAQModule.hpp>
#include <utilities/WorkerThread.hpp>

#include <memory>
#include <string>

namespace dunedaq {
namespace wibmod {

/**
 * @brief ProtoWIBConfigurator is a simple DAQModule implementation that
 * provides a configuration and monitoring interface to the WIB1
 */
class ProtoWIBConfigurator : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief ProtoWIBConfigurator Constructor
   * @param name Instance name for this ProtoWIBConfigurator instance
   */
  explicit ProtoWIBConfigurator(const std::string& name);

  ProtoWIBConfigurator(const ProtoWIBConfigurator&) = delete; ///< ProtoWIBConfigurator is not copy-constructible
  ProtoWIBConfigurator& operator=(const ProtoWIBConfigurator&) =
    delete;                                                         ///< ProtoWIBConfigurator is not copy-assignable
  ProtoWIBConfigurator(ProtoWIBConfigurator&&) = delete;            ///< ProtoWIBConfigurator is not move-constructible
  ProtoWIBConfigurator& operator=(ProtoWIBConfigurator&&) = delete; ///< ProtoWIBConfigurator is not move-assignable

  void init(const data_t&) override;

private:
  std::unique_ptr<WIB> wib;

  // Commands
  void do_conf(const data_t&);
  void do_settings(const data_t&);
  void do_start(const data_t&);
  void do_stop(const data_t&);
  void do_scrap(const data_t&);

  const protowibconfigurator::FEMBSettings& femb_conf_i(const protowibconfigurator::WIBSettings& conf, size_t i);

  void setup_femb_fake_data(size_t iFEMB,
                            const protowibconfigurator::FEMBSettings& FEMB_conf,
                            bool continue_on_reg_read_error);
  void setup_femb(size_t iFEMB, const protowibconfigurator::FEMBSettings& FEMB_conf, bool continue_on_reg_read_error);
};

} // namespace wibmod

} // namespace dunedaq

#endif // WIBMOD_PLUGINS_PROTOWIBCONFIGURATOR_HPP_
