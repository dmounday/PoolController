/*
 * WirelessRT.h
 *
 *  Created on: Jul 26, 2020
 *      Author: dmounday
 */

#ifndef SRC_WIRELESSSENSORS_H_
#define SRC_WIRELESSSENSORS_H_
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <boost/property_tree/ptree.hpp>
#include "SensorModule.h"
#include "RTSensor.h"

namespace SwitchTiming {
namespace pt = boost::property_tree;
//
// The WirelessRT object models a wireless processor controlling 1 or more
// sensors. It is identified by teh wireless_id of the transmitter.
//
class WirelessSensors: public SensorModule {
  int signal_strength_;
  std::vector<RTSensor> sensors_;
	StatusList status;

public:
	/**
	 * Object that represents all the wireless sensors.
	 * Example of property tree:
	 * {"Wireless":
      {
      "_comment4": "Sensor data passed from MightyHat serial port",
       "Sensors":  {
         "PoolTemp":
           {
          "RemoteID":  790,
          "MsgPrefix":  "WC"
           },
         "CaseTemp":
           {
          "RemoteID":  790,
          "MsgPrefix":  "CC"
           },
        "CaseHum":
          {
          "RemoteID":  790,
          "MsgPrefix":  "Hum"
          }
    }
	 * @param name
	 * @param
	 */
	WirelessSensors(boost::asio::io_context&, const std::string& name, const pt::ptree&);

	//int WirelessID()const {return wireless_id_;};
	// Update the sensor data for this RT
	// Data begins with "[id] label:<value> SS:<signal-strength-value>
	void Update(const std::string& data);
	StatusList GetStatusList();
	SensorIDs GetSensorIDs() override;
	float GetSensorValue(std::string const& )const override;

};

} /* namespace WirelessSerialSensors */

#endif /* SRC_WIRELESSSENSORS_H_ */
