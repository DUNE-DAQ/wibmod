/**
 * @file WIBCommon.hpp
 *
 * WIBCommon class used communicate with a WIB
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
 
#include <ers/ers.h>

#include "wibmod/WIBCommon.hpp"
#include "wib.pb.h"

namespace dunedaq {
namespace wibmod {

WIBCommon::WIBCommon(const std::string &wib_addr)
{
    context = new zmq::context_t(1);
    ERS_LOG(wib_addr << " ZMQ context initialized");
    socket = new zmq::socket_t(*context, ZMQ_REQ);
    ERS_LOG(wib_addr << " ZMQ socket initialized");
    socket->connect(wib_addr); // tcp://192.168.121.*:1234
    ERS_LOG(wib_addr << " Connected!");
}

WIBCommon::~WIBCommon()
{
    socket->close();
    delete socket;
    delete context;
}

template <class R, class C>
void 
WIBCommon::send_command(const C &msg, R &repl)
{
    wibproto::Command command;
    command.mutable_cmd()->PackFrom(msg);
    
    std::string cmd_str;
    command.SerializeToString(&cmd_str);
    
    zmq::message_t request(cmd_str.size());
    memcpy((void*)request.data(), cmd_str.c_str(), cmd_str.size());
    socket->send(request);
    
    zmq::message_t reply;
    socket->recv(&reply);
    
    std::string reply_str(static_cast<char*>(reply.data()), reply.size());
    repl.ParseFromString(reply_str);
}

} // namespace wibmod
} // namespace dunedaq
