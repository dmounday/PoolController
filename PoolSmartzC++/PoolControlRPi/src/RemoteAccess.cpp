/*
 * RemoteAccess.cpp
 *
 *  Created on: Feb 15, 2021
 *      Author: dmounday
 */

#include "RemoteAccess.h"

namespace SwitchTiming {

RemoteAccess::RemoteAccess (pt::ptree const& node, GblData& gd):
          localIP_ {node.get<std::string>("LocalIP")},
          localPort_ {node.get<short unsigned>("LocalPort")},
          protocol_ {node.get<std::string>("Protocol", "")},
          gD_{gd}
{};

void
RemoteAccess::Start(){
  PLOG(plog::debug)<< "Starting remote access on "<< localIP_<< ":"<< localPort_;
 std::shared_ptr json_handler = std::make_shared<JsonHandler>(gD_);
 auto request_handler = std::bind(&JsonHandler::RequestHandler, json_handler,
                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

 // Create and launch a listening port ----------------------------------------------------
 auto const address = net::ip::make_address(localIP_);
 std::make_shared<listener>(gD_.IOC(), tcp::endpoint{address, localPort_},request_handler)
     ->run();
}

} /* namespace SwitchTiming */
