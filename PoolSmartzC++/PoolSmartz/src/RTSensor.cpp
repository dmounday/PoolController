/*
 * WirelessSensor.cpp
 *
 *  Created on: Jul 26, 2020
 *      Author: dmounday
 */
#include "plog/Log.h"
//#define DEBUG
#ifdef DEBUG
#define DBG_LOG(x) x
#else
#define DBG_LOG(x)
#endif
#include "RTSensor.h"

namespace WirelessSerialSensors {

RTSensor::RTSensor(const std::string& sensor_id, const std::string& label):
	sensor_id_{sensor_id}, label_{label}
{
}
void RTSensor::Update(const std::string& data){
	DBG_LOG(PLOG(plog::debug)<< data);
	last_reading_ = std::chrono::system_clock::now();
	auto s = data.find_first_not_of(" :");
	auto e = data.find(' ', s);
	if ( e != std::string::npos )
		sample_ = data.substr(s, e-s);
}
} /* namespace WirelessSerialSensors */
