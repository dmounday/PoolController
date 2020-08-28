/*
 * WirelessSensor.h
 *
 *  Created on: Jul 26, 2020
 *      Author: dmounday
 */

#ifndef SRC_RTSENSOR_H_
#define SRC_RTSENSOR_H_

#include <string>
#include <chrono>

namespace WirelessSerialSensors {

class RTSensor {
	std::string sensor_id_;
	std::string label_;
	std::chrono::system_clock::time_point last_reading_;
	std::string sample_;
public:
	// This models the single instance of a sensor, such as temperature,
	// transmitted from a Wireless Id.
	RTSensor(const std::string& sensor_id, const std::string& label);
	inline const std::string& SensorID()const {return sensor_id_;};
	inline const std::string& SensorLabel()const {return label_;};
	inline std::chrono::system_clock::time_point LastReading()const {
		return last_reading_;
	}
	inline void SetLastReading(std::chrono::system_clock::time_point time){
		last_reading_ = time;
	}
	void Update(const std::string& data);
	inline const std::string& LastSample()const {
		return sample_;
	}
};

} /* namespace WirelessSerialSensors */

#endif /* SRC_RTSENSOR_H_ */
