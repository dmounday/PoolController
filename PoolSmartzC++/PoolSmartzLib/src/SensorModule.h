/*
 * Sensor.h
 *
 *  Created on: Jan 19, 2021
 *      Author: dmounday
 */

#ifndef SRC_SENSORMODULE_H_
#define SRC_SENSORMODULE_H_
#include <boost/asio.hpp>
#include "EquipmentBase.h"

namespace SwitchTiming {
using SensorIDs = std::vector<std::string const *>;
class SensorModule: public EquipmentBase {
public:
  SensorModule (boost::asio::io_context& ioc, const std::string& name);
  /**
   * Set refresh_stop flag. May stop refresh threads in derived objects.
   */
  virtual void Stop() override {refresh_stop_ = true;};
  virtual void Start();
  virtual SensorIDs GetSensorIDs() = 0;
  virtual float GetSensorValue(std::string const&) const = 0;
protected:
  bool refresh_stop_;
  boost::asio::io_context& ioc_;
  std::chrono::system_clock::time_point last_reading_;
};

} /* namespace SwitchTiming */

#endif /* SRC_SENSORMODULE_H_ */
