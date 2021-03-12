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
#include "WirelessSensors.h"

namespace SwitchTiming {

WirelessSensors::WirelessSensors(boost::asio::io_context& ioc,
                                 const std::string& name, const pt::ptree& node):
  SensorModule(ioc, name), signal_strength_{0}
{
  const auto& sensors = node.get_child("Sensors");
  for (const auto& s: sensors){
    std::string label = s.first;
    const auto& si = s.second;
    int rf_id = si.get<int>("RemoteID");
    std::string prefix = si.get<std::string>("MsgPrefix");
    sensors_.push_back({rf_id, prefix, label});
  }
}
/**
WirelessSensors::WirelessSensors(const std::string& name, int id, std::vector<RTSensor> sensors):
	EquipmentBase(name),
	wireless_id_{id}, sensors_{sensors}, signal_strength_{0}
{
}
****/
void
WirelessSensors::Update(const std::string& data){
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

SwitchTiming::StatusList
WirelessSensors::GetStatusList(){
	for ( auto& s: sensors_)
		status.push_back(std::make_pair(s.SensorLabel(), s.LastSample()));
	std::ostringstream time_str;
	std::time_t t_c = std::chrono::system_clock::to_time_t(last_reading_);
	time_str << std::put_time(std::localtime(&t_c), "%F %T");
	status.push_back(std::make_pair("Time", time_str.str() ));
	status.push_back(std::make_pair("Signal", std::to_string( signal_strength_)));
	return status;
}

float
WirelessSensors::GetSensorValue(std::string const &id ) const{
  auto p = std::find_if(std::begin(sensors_), std::end(sensors_),
                        [&id](auto p){ return p.SensorID() ==id;});
  return stof(p->LastSample());
}
SensorIDs
WirelessSensors::GetSensorIDs()
{
  SensorIDs ids;
  for(auto& i: sensors_){
    ids.push_back(&i.SensorLabel());
  }
  return ids;
}
} /* namespace WirelessSerialSensors */
