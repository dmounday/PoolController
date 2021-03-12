/*
 * DS18B20.h
 *
 *  Created on: Jan 16, 2021
 *      Author: dmounday
 */

#ifndef SRC_DS18B20_H_
#define SRC_DS18B20_H_
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <boost/property_tree/ptree.hpp>
#include "SensorModule.h"

#include "plog/Log.h"
namespace SwitchTiming {
namespace pt = boost::property_tree;

class DS18B20: public SensorModule {
  const int BAD_SAMPLE {1000000};
  const float BAD_TEMP {1000.0};
public:
  /**
   * DS18B20 property tree. Such as:
   * "DS18B20": {
      "Sample-Interval": 60,
      "path": "/sys/bus/w1/devices",
      "file": "temperature",
      "Sensors":
        {
         "HeatPumpInTemp":
          {
           "ProbeID": "28-3c01b556939e"
          },
        "HeatPumpOutTemp":
          {
          "ProbeID": "28-3c01d6077312"
          }
        }
   *
   * @param
   */
  DS18B20 (boost::asio::io_context&, const std::string& name, const pt::ptree& );
  ~DS18B20();
  void Start() override;
  inline void Stop() override{ refresh_stop_ = true;};
  /**
   *
   * @return vector of ID in this sensor module.
   */
  SensorIDs GetSensorIDs() override;
  float GetSensorValue(std::string const& ) const override;
  StatusList GetStatusList();
private:
  struct ProbeDetail {
    std::string id;
    std::string probeId;
    int sample_value;
    float correction;
  };

  bool refresh_stop_;
  boost::asio::steady_timer sample_timer_;
  int refresh_seconds_;
  std::string sensor_path_;
  std::string data_file_;
  std::vector<ProbeDetail> probes_;
  void RepeatSample();
  void ReadSensors();
  void RefreshThread();


};

} /* namespace SwitchTiming */

#endif /* SRC_DS18B20_H_ */
