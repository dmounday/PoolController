/*
 * SensorDope.cpp
 *
 *  Created on: Feb 9, 2021
 *      Author: dmounday
 */

#include "SensorDope.h"

namespace SwitchTiming {

SensorDope::SensorDope (GblData& gD,
                        const pt::ptree &sensors) {
  try {
    const auto &wirelessProp = sensors.get_child (WIRELESS);
    auto wireless = std::make_shared<WirelessSensors> (gD.IOC(), WIRELESS,
                                                       wirelessProp);
    gD.AddEquipment(WIRELESS, std::dynamic_pointer_cast<EquipmentBase> (wireless));
    MapIDs(wireless->GetSensorIDs(), gD.Equip(WIRELESS));

    const auto &si7021tree = sensors.get_child (SI7021S);
    auto si7021p = std::make_shared<SI7021> (gD.IOC(), SI7021S, si7021tree);
    gD.AddEquipment(SI7021S, std::dynamic_pointer_cast<EquipmentBase> (si7021p));
    MapIDs(si7021p->GetSensorIDs(), gD.Equip(SI7021S));

    const auto &ds18b20tree = sensors.get_child (DS18B20S);
    auto ds18b20p = std::make_shared<DS18B20> (gD.IOC(), DS18B20S, ds18b20tree);
    gD.AddEquipment(DS18B20S, std::dynamic_pointer_cast<EquipmentBase> (ds18b20p));
    MapIDs(ds18b20p->GetSensorIDs(), gD.Equip(DS18B20S));

  } catch (pt::ptree_error &e) {
    std::cerr << "Sensor config error: " << e.what () << std::endl;
    throw;
  } catch (std::range_error &e) {
    PLOG(plog::error) << "Unknow exception:" << e.what ();
    throw;
  }

}
/**
SensorDope::SensorDope (boost::asio::io_context &ioc, Equipment &equipment,
                        const pt::ptree &sensors) {
  try {
    const auto &wirelessProp = sensors.get_child (WIRELESS);
    auto wireless = std::make_shared<WirelessSensors> (ioc, WIRELESS,
                                                       wirelessProp);
    equipment[WIRELESS] = std::dynamic_pointer_cast<EquipmentBase> (wireless);
    MapIDs(wireless->GetSensorIDs(), equipment[WIRELESS]);
    const auto &si7021tree = sensors.get_child (SI7021S);
    auto si7021p = std::make_shared<SI7021> (ioc, SI7021S, si7021tree);
    equipment[SI7021S] = std::dynamic_pointer_cast<EquipmentBase> (si7021p);
    MapIDs(si7021p->GetSensorIDs(), equipment[SI7021S]);
    const auto &ds18b20tree = sensors.get_child (DS18B20S);
    auto ds18b20p = std::make_shared<DS18B20> (ioc, DS18B20S, ds18b20tree);
    equipment[DS18B20S] = std::dynamic_pointer_cast<EquipmentBase> (ds18b20p);
    MapIDs(ds18b20p->GetSensorIDs(), equipment[DS18B20S]);
  } catch (pt::ptree_error &e) {
    std::cerr << "Sensor config error: " << e.what () << std::endl;
    throw;
  } catch (std::exception &e) {
    PLOG(plog::error) << "Unknow exception:" << e.what ();
  }

}
**/
void
SensorDope::MapIDs(const SensorIDs& ids, std::shared_ptr<EquipmentBase> sensor_module){
  for (auto* i: ids){
    sensor_[*i] = std::dynamic_pointer_cast<SensorModule>(sensor_module);
  }
}
SensorDope::~SensorDope () {
}
std::shared_ptr<SensorModule>
SensorDope::Sensor(std::string const & id){
  return sensor_.at(id);
}
} /* namespace SwitchTiming */
