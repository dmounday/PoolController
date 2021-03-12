/*
 * HeatPumpSchedule.h
 *
 *  Created on: Feb 11, 2021
 *      Author: dmounday
 */

#ifndef HEATPUMPSCHEDULE_H_
#define HEATPUMPSCHEDULE_H_

#include "GblData.h"
#include "RelaySwitcher.h"
#include "EquipSched.h"

namespace SwitchTiming {

class HeatPumpSchedule : public EquipSched {
  const int DEFAULT_INTERVAL{60};
  enum class State {
    OFF,
    ON
  };
public:
  HeatPumpSchedule (pt::ptree& sched_tree, GblData& );
  void SetSensor(const pt::ptree& sensor_prop) override;
  void Run();
private:
  GblData const& gD_;
  boost::asio::system_timer sTimer_;
  pt::ptree& sensor_prop_;
  std::string sensorId_;
  int sensorDelay_;
  int sensorInterval_;
  int temp_;
  float diff_;
  State state_;

  void  ReSample(const boost::system::error_code &ec);
};

} /* namespace SwitchTiming */

#endif /* HEATPUMPSCHEDULE_H_ */
