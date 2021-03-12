/*
 * SI7021.cpp
 *
 *  Created on: Jul 11, 2020
 *      Author: dmounday
 */

#include "SI7021.h"

//#define DEBUG
#ifdef DEBUG
#define DBG_LOG(x) x
#else
#define DBG_LOG(x)
#endif
namespace SwitchTiming {


SI7021::SI7021(boost::asio::io_context& ioc, const std::string& name, const pt::ptree& prop):
    SensorModule{ioc, name},
    interval_{prop.get<int>("Sample-Interval", 0)},
    refresh_stop_{false},
    path_{prop.get<std::string> ("Path")},
    i2c(path_, SI7021_I2CADDR),
    sample_timer_(ioc)
{
  try{
    const auto& sensors = prop.get_child("Sensors");
    for (const auto& s: sensors){
      const auto& si = s.second;
      sensors_.push_back (
       {s.first, si.get<std::string>("Type"), 0, si.get<float>("Correction", 0.0)});
    }
  } catch (pt::ptree_error& e){
    PLOG(plog::error)<< "Sensor configuration error";
    throw;
  } catch (std::runtime_error& e){
    PLOG(plog::error)<< "Unable to open I2C interface: "<< e.what();
  }
}

void SI7021::Start()
{
  Reset();
  Sample();
  sample_timer_.expires_after(std::chrono::seconds(interval_));
  sample_timer_.async_wait(std::bind(&SI7021::RepeatSample, this));
}
void SI7021::RepeatSample(){
  if ( !refresh_stop_ ){
    Sample();
    sample_timer_.expires_at(sample_timer_.expiry() + std::chrono::seconds(interval_));
    sample_timer_.async_wait(std::bind(&SI7021::RepeatSample, this));
  }
}

void SI7021::Sample(){
  last_reading_ = std::chrono::system_clock::now();
	auto temp = ReadTemp();
	auto p = std::find_if(std::begin(sensors_), std::end(sensors_),
	                      [](auto p){ return p.type=="temperature";});
	p->sample_value = (temp * (9.0 / 5.0) + 32) + p->correction; // make F.wsa
	auto hum = ReadHumidity();
  p = std::find_if(std::begin(sensors_), std::end(sensors_),
                        [](auto p){ return p.type=="humidity";});
  p->sample_value = hum;
  DBG_LOG(PLOG(plog::debug) << "Temperature: " << temp <<
			" C  Humidity: " << hum);
}
void SI7021::Reset(){
	uint8_t data {SI7021_RESET};
	i2c.write(&data, 1);
  boost::asio::steady_timer tmr(ioc_, boost::asio::chrono::milliseconds(50));
 tmr.wait();

	data = i2c.regread8(SI7021_READREG);
	if ( data != 0x3a ) {
		PLOG(plog::error)<< "Reset failed: "<< data;
	}
}
float SI7021::ReadTemp(){
	uint8_t data {SI7021_READTEMP};
	i2c.write(&data, 1);
	 boost::asio::steady_timer tmr(ioc_, boost::asio::chrono::milliseconds(50));
	tmr.wait();
	uint8_t t[2];
	i2c.read(t, 2);
  float temp = (t[0]<<8) |(t[1]&0xfc);

	temp *= 175.72f;
	temp /= 65536.0f;
	temp -= 46.85f;
	return temp;
}

float SI7021::ReadHumidity(){
	uint8_t cmd {SI7021_READHUM};
	i2c.write(&cmd, 1);
  boost::asio::steady_timer tmr(ioc_, boost::asio::chrono::milliseconds(50));
  tmr.wait();
	uint8_t h[2];
	i2c.read(h, 2);
	return (((h[0]*256 +h[1])* 125.0)/65536.0)-6;
}

SwitchTiming::StatusList
SI7021::GetStatusList(){
	SwitchTiming::StatusList status;
	for ( const auto& s: sensors_){
	  status.push_back(std::make_pair(s.id, std::to_string(s.sample_value)));
	}
  std::ostringstream time_str;
  std::time_t t_c = std::chrono::system_clock::to_time_t(last_reading_);
  time_str << std::put_time(std::localtime(&t_c), "%F %T");
  status.push_back(std::make_pair("Time", time_str.str() ));
	return status;
}

float
SI7021::GetSensorValue(std::string const& id) const{
  auto p = std::find_if(std::begin(sensors_), std::end(sensors_),
                        [&id](auto p){ return p.id == id;});
  return p->sample_value;
}
SensorIDs SI7021::GetSensorIDs () {
  SensorIDs ids;
  for (auto& i: sensors_){
    ids.push_back(&i.id);
  }
  return ids;
}
} /* namespace SwitchTiming */

