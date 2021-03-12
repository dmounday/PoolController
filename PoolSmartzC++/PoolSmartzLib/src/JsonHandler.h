/*
 * JsonHandler.h
 *
 *  Created on: Feb 5, 2021
 *      Author: dmounday
 */

#ifndef SRC_JSONHANDLER_H_
#define SRC_JSONHANDLER_H_
#include <memory>

#include "HTTPServerLib.h"
#include "GblData.h"
#include "EquipmentBase.h"
#include "EquipmentCommands.h"
#include "EquipmentStatus.h"
#include "ConfigScheduling.h"

namespace SwitchTiming {

class JsonHandler: public std::enable_shared_from_this<JsonHandler> {
  static constexpr char CMD_SET[] ="/set";
  static constexpr char CMD_SETSCHED[] = "/setSched";
  static constexpr char REQ_STATUS[]="/status";
  static constexpr char REQ_SCHED[] = "/sched";
  static constexpr char CMD_SETSENSORS[] = "/setSensors";
  static constexpr std::size_t STATUS_LTH = std::char_traits<char>::length(REQ_STATUS);
  static constexpr std::size_t SCHED_LTH = std::char_traits<char>::length(REQ_SCHED);

public:
  JsonHandler (GblData const & gD);
  virtual ~JsonHandler ();
  std::string PostHandler(beast::string_view path, beast::string_view json);
  std::string GetHandler(beast::string_view path);
  std::string RequestHandler(http::verb, beast::string_view path, beast::string_view);
  std::shared_ptr<JsonHandler> getptr();
private:
  std::string result_;
  GblData const & gD_;
};

} /* namespace SwitchTiming */

#endif /* SRC_JSONHANDLER_H_ */
