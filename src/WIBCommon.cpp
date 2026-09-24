/**
 * @file WIBCommon.hpp
 *
 * WIBCommon class used communicate with a WIB
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "wibmod/WIBCommon.hpp"

#include "logging/Logging.hpp"

#include <string>

namespace dunedaq {
namespace wibmod {

WIBCommon::WIBCommon(const std::string& wib_addr)
  : context(1)
  , socket(context, ZMQ_REQ)
{
  socket.connect(wib_addr); // tcp://192.168.121.*:1234
  TLOG_DEBUG(0) << wib_addr << " Connected!";
}

WIBCommon::~WIBCommon()
{
  socket.close();
}

} // namespace wibmod
} // namespace dunedaq
