/*
 * SI7021.h
 *
 *  Created on: Jul 11, 2020
 *      Author: dmounday
 */

#ifndef SRC_SI7021_H_
#define SRC_SI7021_H_
#include <iostream>
#include <string>
#include <iomanip>
#include <boost/property_tree/ptree.hpp>
#include "plog/Log.h"
#include "SensorModule.h"
#include "cppgpio/i2c.hpp"

namespace SwitchTiming {
namespace pt = boost::property_tree;
class SI7021: public SensorModule {
  static const unsigned int SI7021_I2CADDR {0x40};
  static const uint8_t SI7021_READTEMP {0xF3};  // Read Temp, No Hold Master mode.
  static const uint8_t SI7021_READHUM {0xF5};   // Read RH, NO Hold Master mode.
  static const uint8_t SI7021_WRITEREG {0xE6};  // Write RH/T user Register 1
  static const uint8_t SI7021_READREG {0xE7};   // Read RH/T User Register 1
  static const uint8_t SI7021_RESET  {0xFE};    // Reset

public:
  /**
   * Represents the SI7021 sensor on the I2C bus of the controller(RPi).
   * Property example:
   "SI7021": {
      "Sample-Interval": 60,
      "Path": "/dev/i2c-1",
      "_comment5": "Sensor in controller cabinet to monitor temp and hum.",
      "Sensors":
      {
        "ControllerTemp":
         {"Type": "temperature",
          "Correction": 0.0},
        "ControllerHum":
         {"Type": "humidity"}
      }
    },
   * @param name
   * @param
   */

  SI7021(boost::asio::io_context&, const std::string& name, const pt::ptree&);
  void Start() override;
  void Stop() override {refresh_stop_=true;};


	StatusList GetStatusList();
  SensorIDs GetSensorIDs() override;
  float GetSensorValue(std::string const& )const override;
private:
  int interval_;
	bool refresh_stop_;
	std::string path_;
  GPIO::I2C i2c;

	boost::asio::steady_timer sample_timer_;
  float ReadTemp();
  float ReadHumidity();
  void Reset();
  void RepeatSample();
	void Sample();
	struct SensorDetail{
	  std::string id;
	  std::string type;
	  float sample_value;
	  float correction;
	};
	std::vector<SensorDetail> sensors_;

};

} /* namespace SwitchTiming */

#endif /* SRC_SI7021_H_ */
