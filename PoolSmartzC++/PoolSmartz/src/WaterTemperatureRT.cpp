/*
 * WaterTemperature.cpp
 *
 *  Created on: Jul 27, 2020
 *      Author: dmounday
 */

#include "WaterTemperatureRT.h"

namespace WirelessSerialSensors {

WaterTemperatureRT::WaterTemperatureRT(const std::string& name, const int wireless_id):
	WirelessRT(name, wireless_id,
		{{"WC", "Water"},
		{"CC", "Case Temp"},
		{"Hum", "Case Humidity"}})
{}

} /* namespace WirelessSerialSensors */
