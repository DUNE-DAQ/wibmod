/**
 * @file WIBCommon.hpp
 *
 * WIBCommon class used communicate with a WIB
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef WIBMOD_INCLUDE_WIBMOD_WIBCOMMON_HPP_
#define WIBMOD_INCLUDE_WIBMOD_WIBCOMMON_HPP_

#include "logging/Logging.hpp"

#include "wib.pb.h"
#include "zmq.hpp"

#include <string>

namespace dunedaq {
namespace wibmod {

/**
 * @brief The WIBCommon class defines convenience methods for using the ZeroMQ
 * and protobuf communication protocol to the WIB
 */
class WIBCommon
{
public:
  WIBCommon(const std::string& wib_addr);

  ~WIBCommon();

  template<class R, class C>
  void send_command(const C& msg, R& repl);

private:
  zmq::context_t context;

  zmq::socket_t socket;
};

template<class R, class C>
void
WIBCommon::send_command(const C& msg, R& repl)
{
  wib::Command command;
  command.mutable_cmd()->PackFrom(msg);

  std::string cmd_str;
  command.SerializeToString(&cmd_str);

  zmq::message_t request(cmd_str.size());
  memcpy(static_cast<void*>(request.data()), cmd_str.c_str(), cmd_str.size());
  socket.send(request);

  zmq::message_t reply;
  socket.recv(&reply);

  std::string reply_str(static_cast<char*>(reply.data()), reply.size());
  repl.ParseFromString(reply_str);
}

} // namespace wibmod
} // namespace dunedaq

#endif // WIBMOD_INCLUDE_WIBMOD_WIBCOMMON_HPP_
