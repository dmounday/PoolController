/*
 * SensorDope.h
 *
 *  Created on: Feb 9, 2021
 *      Author: dmounday
 */

#ifndef SRC_SENSORDOPE_H_
#define SRC_SENSORDOPE_H_
#include <memory>

#include "plog/Log.h"
#include <boost/property_tree/ptree.hpp>
#include "GblData.h"
#include "EquipmentBase.h"
#include "WirelessSensors.h"
#include "DS18B20.h"
#include "SI7021.h"

namespace SwitchTiming {
namespace pt = boost::property_tree;

const std::string WIRELESS{"Wireless"};
const std::string SI7021S{"SI7021"};
const std::string DS18B20S{"DS18B20"};

class SensorDope {
public:
  /**
   * SensorDope creates the various sensors based on the property tree.
   * A property tree example:
   *{"Wireless":
      {
      "_comment4": "Sensor data passed from MightyHat serial port",
       "Sensors":  {
         "PoolTemp":
           {
          "RemoteID":  790,
          "MsgPrefix":  "WC"
           },
         "CaseTemp":
           {
          "RemoteID":  790,
          "MsgPrefix":  "CC"
           },
        "CaseHum":
          {
          "RemoteID":  790,
          "MsgPrefix":  "Hum"
          }
    }
   * @param Boost ASIO io executor.
   * @param Reference to Equipment vector.
   * @param boost::property_tree (child of Sensors node).
   */
  //SensorDope (boost::asio::io_context&, Equipment&,
  //            const pt::ptree& );
  SensorDope (GblData& gD, const pt::ptree &sensors);
  virtual ~SensorDope ();
  /**
   * Return the shared_ptr to the SensorModule that monitors
   * the <id> sensor.
   * @param id
   * @return
   */
  std::shared_ptr<SensorModule> Sensor(std::string const & id);
private:
  void MapIDs(const SensorIDs& , std::shared_ptr<EquipmentBase> );
  // A SensorModule may contain one or more sensors or probes. sensor_
  // maps the probes name to the sensor_module to which it belongs.
  std::map<std::string, std::shared_ptr<SensorModule>> sensor_;
};

} /* namespace SwitchTiming */

#endif /* SRC_SENSORDOPE_H_ */
