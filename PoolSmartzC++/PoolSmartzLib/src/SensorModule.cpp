/*
 * Sensor.cpp
 *
 *  Created on: Jan 19, 2021
 *      Author: dmounday
 */

#include "SensorModule.h"

namespace SwitchTiming {

SensorModule::SensorModule (boost::asio::io_context& ioc, const std::string& name):
  EquipmentBase{name}, refresh_stop_{false}, ioc_{ioc}
{}
void SensorModule::Start(){
  PLOG(plog::debug);
  return;
}
} /* namespace SwitchTiming */
