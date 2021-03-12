/*
 * DS18B20Master.cpp
 *
 *  Created on: Jan 15, 2021
 *      Author: dmounday
 */
#include <thread>
#include "DS18B20Master.h"

namespace SwitchTiming {
namespace fs = std::filesystem;

DS18B20Master::DS18B20Master (int refresh_seconds) :
    refresh_seconds_ { refresh_seconds },refresh_stop_{false} {
  fs::path p { DEVICE_PATH };
  try {
    if (fs::exists (p) && fs::is_directory (p)) {
      FindFiles (p, [] (const fs::path &p) {
        auto filename = p.string ();
        return filename.find ("28-") != std::string::npos;
      });
    } else {
      std::cerr << p << " does not exist." << std::endl;
    }
  } catch (const fs::filesystem_error &e) {
    std::cerr << e.what () << std::endl;
  }
  ReadSensors ();
  if ( refresh_seconds> 0 ){
    // start the refresh thread.
    reader_ = std::thread(&DS18B20Master::RefreshThread, this );
  }
}

int DS18B20Master::FindFiles (const fs::path &dir,
                              std::function<bool (const fs::path&)> filter) {
  if (fs::exists (dir)) {
    for (auto const &entry : fs::directory_iterator (dir)) {
      if (fs::is_directory (entry) && filter (entry)) {
        sensor_dirs.push_back (entry);
      }
    }
  }
  return sensor_dirs.size ();
}
void DS18B20Master::ReadSensors () {
  for (auto s : sensor_dirs) {
    std::string sensor_id = s.filename();
    s /= DATA_FILE;
    std::ifstream infile (s);
    std::string linebuf;
    if (infile.is_open ()) {
      if (std::getline (infile, linebuf)) {
        if (isdigit (linebuf[0])) {
          int num = std::stoi (linebuf);
          readings_[sensor_id] = num;
          infile.close();
          continue;
        }
      }
      readings_[sensor_id] = BAD_TEMP;
    }
  }
}

void DS18B20Master::RefreshThread()
{
  while ( !refresh_stop_ ){
    std::this_thread::sleep_for(std::chrono::seconds(refresh_seconds_));
    ReadSensors();
  }
}
float DS18B20Master::GetTempC (const std::string &name) {
  float temp = readings_.at (name);
  return temp / 1000;
}

float DS18B20Master::GetTempF (const std::string &name) {
  float temp = readings_.at (name);
  temp /= 1000.0;
  return temp * (9.0 / 5.0) + 32.0;
}
const std::vector<std::string> DS18B20Master::GetSensorNames () const {
  std::vector<std::string> names;
  for (auto n : readings_) {
    names.push_back (n.first);
  }
  return names;
}

DS18B20Master::~DS18B20Master()
{
  refresh_stop_ = true;
  reader_.join();
}
} /* namespace SwitchTiming */
