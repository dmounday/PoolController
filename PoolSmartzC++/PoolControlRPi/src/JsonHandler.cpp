/*
 * JsonHandler.cpp
 *
 *  Created on: Feb 5, 2021
 *      Author: dmounday
 */

#include "JsonHandler.h"

namespace SwitchTiming {

JsonHandler::JsonHandler (GblData const& gD):
  gD_{gD}
{
}

std::string JsonHandler::PostHandler(beast::string_view path, beast::string_view json) {
  PLOG(plog::debug) << "path: " << path;
  PLOG(plog::debug) << "Json request: " << json;
  try {
    if ( path == CMD_SET){
    EquipmentCommands eq_cmd(path, json );
    if ( eq_cmd.RunCommand(gD_.AllEquipment()))
      return std::string("Success");
    return std::string("Failure");
    } else if ( path == CMD_SETSCHED){
      auto& schedules = gD_.Scheduling();
      return schedules.SetSchedules(json);
    } else if ( path == CMD_SETSENSORS){
      auto& schedules = gD_.Scheduling();
      return schedules.SetSensors(json);
    }
  } catch (std::exception &e){
    PLOG(plog::error)<< "Exception handling POST: "<< e.what();
    return std::string("JSON format error!");
  }
  return std::string("Failure");
}

std::string JsonHandler::GetHandler(beast::string_view path) {
  if ( path.starts_with(REQ_STATUS)){
    EquipmentStatus eq_status(gD_, path);
    return eq_status();
  } else if (path.starts_with(REQ_SCHED)){
    auto& schedules = gD_.Scheduling();
    return schedules.GetSchedules();
  }
  return std::string{};
}
std::string JsonHandler::RequestHandler(http::verb method, beast::string_view path,
                                        beast::string_view body_str) {
  if (method == http::verb::get)
    return GetHandler(path);
  else
    return PostHandler(path, body_str);
}
std::shared_ptr<JsonHandler> JsonHandler::getptr() {
  return shared_from_this();
}
JsonHandler::~JsonHandler () {
}

} /* namespace SwitchTiming */
