
#ifndef WIBMOD_INCLUDE_WIBMOD_WIBCOMMON_HPP_
#define WIBMOD_INCLUDE_WIBMOD_WIBCOMMON_HPP_
/**
 * @file WIBCommon.hpp
 *
 * WIBCommon class used communicate with a WIB
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <string>
#include <zmq.hpp>

namespace dunedaq {
namespace wibmod {

/**
 * @brief The WIBCommon class defines convenience methods for using the ZeroMQ
 * and protobuf communication protocol to the WIB
 */
class WIBCommon 
{
public:

  WIBCommon(const std::string &wib_addr);

  ~WIBCommon();
  
  template <class R, class C>
  void send_command(const C &msg, R &repl); 

private:

  zmq::context_t *context;

  zmq::socket_t *socket;

};
 
} // namespace wibmod
} // namespace dunedaq

#endif // DFMODULES_INCLUDE_DFMODULES_STORAGEKEY_HPP_
