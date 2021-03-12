/*
 * ConfigScheduling.h
 *
 *  Created on: Feb 10, 2021
 *      Author: dmounday
 */

#ifndef CONFIGSCHEDULING_H_
#define CONFIGSCHEDULING_H_
#include <boost/bind/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/utility/string_view.hpp>
using namespace boost::placeholders;
#include "GblData.h"
#include "MainPumpSchedule.h"
#include "HeatPumpSchedule.h"
#include "IC40.h"


namespace SwitchTiming {


class ConfigScheduling {
public:
  /**
   * ConfigScheduling is an object containing the scheduling objects for eqch of
   * the pieces of equipment.
   * @param ioc
   * @param props property tree for Scheduling.
   * @param Equipment vector.
   * Property tree sample
   *
   *"MainPump":
      {
      "Start": "08:00",
      "Stop": "20:00",
      "Settings": {
        "Speed": 1
        }
      }
      },
      "IC40":
        {
        "Start": "08:10",
        "Stop": "20:00",
        "Settings": {
          "Rate": "30"
        }
        },
      "Booster":
        {
        "Start": "manual"
        },
      "Lighting":
        {
        "Start": "manual"
        },
    "HeatPump":{
      "Start": "08:05",
      "Stop": "20:00",
      "Sensor": {
       "ID": "HeatPumpInTemp",
       "Delay": "120",
        "TempOn": "88",
        "TempOff": "90"
      }
    }
   */
  ConfigScheduling (pt::ptree& props, GblData& );

  void RunSchedule();
  std::string GetSchedules();
  std::string SetSchedules(boost::string_view );
  std::string SetSensors(boost::string_view);
private:
  GblData const& gD_;
  std::map<std::string, std::unique_ptr<EquipSched>> sched_;

};

} /* namespace SwitchTiming */

#endif /* CONFIGSCHEDULING_H_ */
