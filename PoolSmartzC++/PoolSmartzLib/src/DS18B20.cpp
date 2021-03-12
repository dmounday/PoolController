/*
 * DS18B20.cpp
 *
 *  Created on: Jan 16, 2021
 *      Author: dmounday
 */

#include "DS18B20.h"
//#define DEBUG
#ifdef DEBUG
#define DBG_LOG(x) x
#else
#define DBG_LOG(x)
#endif
namespace SwitchTiming {
/**
 * Starts a thread to read the DS18B20 probe with probe_id every
 * refresh_sec. A separate thread is started due to the long conversion
 * time required to read the 1 Wire probes.
 * @param probe_id
 * @param refresh_sec
 */

DS18B20::DS18B20 (boost::asio::io_context &ioc, const std::string &name,
                  const pt::ptree &prop) :
    SensorModule { ioc, name }, refresh_stop_ { false }, sample_timer_ { ioc } {
  PLOG(plog::debug);
  sensor_path_ = prop.get<std::string> ("Path");
  data_file_ = prop.get<std::string> ("File");
  refresh_seconds_ = prop.get<int> ("Sample-Interval");
  const auto &sensors = prop.get_child ("Sensors");
  for (const auto &s : sensors) {
    const auto &si = s.second;
    probes_.push_back ( { s.first, si.get<std::string> ("ProbeID"), 0,
                  si.get<float>("Correction", 0.0)});
  }

}

DS18B20::~DS18B20()
{
  refresh_stop_ = true;
}

void DS18B20::Start()
{
  PLOG(plog::debug);
  ReadSensors();
  sample_timer_.expires_after(std::chrono::seconds(refresh_seconds_));
  sample_timer_.async_wait(std::bind(&DS18B20::RepeatSample, this));
}

void DS18B20::RepeatSample(){
  if ( !refresh_stop_ ){
    ReadSensors();
    sample_timer_.expires_at(sample_timer_.expiry() + std::chrono::seconds(refresh_seconds_));
    sample_timer_.async_wait(std::bind(&DS18B20::RepeatSample, this));
  }
}
void DS18B20::ReadSensors () {
  last_reading_ = std::chrono::system_clock::now();
  for (auto &p : probes_) {
    std::string path = sensor_path_ + '/' + p.probeId + '/' + data_file_;
    std::ifstream infile (path);
    std::string linebuf;
    if (infile.is_open ()) {
      if (std::getline (infile, linebuf)) {
        if (isdigit (linebuf[0])) {
          p.sample_value = std::stoi (linebuf);
          infile.close ();
          DBG_LOG(PLOG(plog::debug)<< p.probeId << ": "<< p.sample_value);
          continue;
        }
      }
      p.sample_value = BAD_SAMPLE;
      infile.close ();
    } else
      p.sample_value = BAD_SAMPLE;
    PLOG(plog::error)<< "Bad Sample: "<< path;
  }
}


StatusList DS18B20::GetStatusList () {
  StatusList status;
  for (auto &p : probes_) {
    float temp = p.sample_value;
    if (p.sample_value < BAD_SAMPLE) {
      temp /= 1000.0;
      temp = temp * (9.0 / 5.0) + 32;
      temp = temp + p.correction;
    }
    status.push_back (std::make_pair (p.id, std::to_string (temp)));
  }
  std::ostringstream time_str;
  std::time_t t_c = std::chrono::system_clock::to_time_t (last_reading_);
  time_str << std::put_time (std::localtime (&t_c), "%F %T");
  status.push_back (std::make_pair ("Time", time_str.str ()));
  return status;
}

float DS18B20::GetSensorValue (std::string const &id) const {
  auto p = std::find_if (std::begin (probes_), std::end (probes_),
                         [&id] (auto p) {
                           return p.id == id;
                         });
  if (p != probes_.end ()) {
    float temp = p->sample_value;
    temp /= 1000.0;
    temp = temp * (9.0 / 5.0) + 32;
    temp = temp + p->correction;
    return temp;
  }
  return 0.0;
}
SensorIDs
DS18B20::GetSensorIDs(){
  SensorIDs ids;
  for (auto& i: probes_){
    ids.push_back(&i.id);
  }
  return ids;
}
} /* namespace SwitchTiming */
