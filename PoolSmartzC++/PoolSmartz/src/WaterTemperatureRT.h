/*
 * WaterTemperature.h
 *
 *  Created on: Jul 27, 2020
 *      Author: dmounday
 */

#ifndef SRC_WATERTEMPERATURE_H_
#define SRC_WATERTEMPERATURE_H_

#include "WirelessRT.h"

namespace WirelessSerialSensors {
//
// This WirelessRT contains a DB18B20 temperature sensor for the water temperature and
// a SI7021 temp/humidity sensor that monitors the environment inside the in-water
// enclosure.
class WaterTemperatureRT: public WirelessRT {

public:
	// Id is the transmitter identifier.
	WaterTemperatureRT(const std::string& name, const int id);

};

} /* namespace WirelessSerialSensors */

#endif /* SRC_WATERTEMPERATURE_H_ */
