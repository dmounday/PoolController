/*
 * WirelessRT.cpp
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
#include "WirelessRT.h"

namespace WirelessSerialSensors {

WirelessRT::WirelessRT(const std::string& name, int id, std::vector<RTSensor> sensors):
	EquipmentBase(name),
	wireless_id_{id}, sensors_{sensors}, signal_strength_{0}
{
}
void
WirelessRT::Update(const std::string& data){
	last_reading_ = std::chrono::system_clock::now();
	auto s = data.find_first_not_of(" ");
	auto e = data.find(':');
	std::string id = data.substr(s, e-s);
	for ( auto& sensor: sensors_){
		DBG_LOG(PLOG(plog::debug)<< "id: " << id << " target? " << sensor.SensorID());
		if ( sensor.SensorID() == id){
			sensor.Update( data.substr(e) );
			break;
		}
	}
	s = data.find("SS:");
	if ( s != std::string::npos )
		signal_strength_ = std::stoi(&data[s+3]);
}

SwitchTiming::StateList
WirelessRT::GetStateList(){
	for ( auto& s: sensors_)
		states.push_back(std::make_pair(s.SensorLabel(), s.LastSample()));
	std::ostringstream time_str;
	std::time_t t_c = std::chrono::system_clock::to_time_t(last_reading_);
	time_str << std::put_time(std::localtime(&t_c), "%F %T");
	states.push_back(std::make_pair("Time", time_str.str() ));
	states.push_back(std::make_pair("Signal", std::to_string( signal_strength_)));
	return states;
}

} /* namespace WirelessSerialSensors */
