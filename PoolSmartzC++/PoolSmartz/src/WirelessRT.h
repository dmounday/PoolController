/*
 * WirelessRT.h
 *
 *  Created on: Jul 26, 2020
 *      Author: dmounday
 */

#ifndef SRC_WIRELESSRT_H_
#define SRC_WIRELESSRT_H_
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include "EquipmentBase.h"
#include "RTSensor.h"

namespace WirelessSerialSensors {
//
// The WirelessRT object models a wireless processor controlling 1 or more
// sensors. It is identified by teh wireless_id of the transmitter.
//
class WirelessRT: public SwitchTiming::EquipmentBase {
	const int wireless_id_;
	std::vector<RTSensor> sensors_;
	std::chrono::system_clock::time_point last_reading_;
	int	signal_strength_;
	SwitchTiming::StateList states;

public:
	WirelessRT(const std::string& name, const int wireless_id, std::vector<RTSensor> sensors);
	int WirelessID()const {return wireless_id_;};
	// Update the sensor data for this RT
	// Data begins with "[id] label:<value> SS:<signal-strength-value>
	void Update(const std::string& data);
	SwitchTiming::StateList GetStateList();


};

} /* namespace WirelessSerialSensors */

#endif /* SRC_WIRELESSRT_H_ */
