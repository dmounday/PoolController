/*
 * DS18B20Master.h
 *
 *  Created on: Jan 15, 2021
 *      Author: dmounday
 */

#ifndef DS18B20MASTER_H_
#define DS18B20MASTER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <map>
#include <functional>
#include <thread>


namespace SwitchTiming {
namespace fs = std::filesystem;
static const char DEVICE_PATH[] ="/sys/bus/w1/devices";
static const std::string DATA_FILE {"temperature"};
// 28 is the ROM code for the DS18B20's 1-wire family
// Prefix on directory name containing sensor data and params.
static const std::string DS18B20_CODE {"28-"};
/**
 * Wrapper class to access the DS18B20 temperature probe using
 * the 1-Wire device interface.
 */
class DS18B20Master {
  const int BAD_TEMP {1000}; // value for returning bad temperature
                            // DS18B20 max temp is 125C

public:
  /**
   * Construct the object with the refresh interval. Note that short
   * intervals cause self-heating of the probes.
   * @param refresh_seconds
   */
  DS18B20Master (int refresh_seconds);
  virtual ~DS18B20Master();
  /**
   * Return last temperature read reading for sensor 'name'.
   * @return temperature in C.
   */
  float GetTempC(const std::string& name );
  /**
   * Return last temperature read reading for sensor 'name'.
   * @return temperature in F.
   */
  float GetTempF(const std::string& name );
  /**
   * Force a immediate refresh of the in memory temperatures.
   */
  void inline Refresh() { ReadSensors(); };
  /**
   * Return a vector of the probes serial numbers (Names).
   */
  const std::vector<std::string>
  GetSensorNames() const;

private:
  std::vector<fs::path> sensor_dirs;
  std::map<std::string, int> readings_;
  int refresh_seconds_;
  std::thread reader_;
  bool refresh_stop_;

  int FindFiles(const fs::path&, std::function<bool(const fs::path&)> filter);
  void ReadSensors();
  void RefreshThread();
};

} /* namespace SwitchTiming */

#endif /* DS18B20MASTER_H_ */
