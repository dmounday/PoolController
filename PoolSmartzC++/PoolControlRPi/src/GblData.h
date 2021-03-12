/*
 * GblData.h
 *
 *  Created on: Feb 12, 2021
 *      Author: dmounday
 */

#ifndef SRC_ROOTDATA_H_
#define SRC_ROOTDATA_H_
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include "EquipmentBase.h"


namespace SwitchTiming {
namespace pt = boost::property_tree;
class GblDataImpl;

class SensorDope;
class ConfigScheduling;
class RemoteAccess;
class LogicLevelShifter;
/**
 * GblData is a wrapper class around the configuration and
 * scheduling objects.
 */
class GblData {
public:
  GblData (boost::asio::io_context& ioc);
  EquipmentPtr Sensor(std::string const& id) const;
  float GetSensorValue(std::string const& id) const;
  EquipmentPtr Equip(std::string const& id) const;
  Equipment const& AllEquipment() const;
  std::shared_ptr<RemoteAccess> GetRemoteAccess() const;
  boost::asio::io_context& IOC() const;
  pt::ptree& RunProperties() const;
  ConfigScheduling& Scheduling()const;
  void
  SetLogicShifter (std::shared_ptr<LogicLevelShifter> ls);
  void
  SetSensorDope (std::shared_ptr<SensorDope> sd);
  void
  SetRemoteAccess (std::shared_ptr<RemoteAccess> ra);
  void
  SetSchedConfig (std::shared_ptr<ConfigScheduling> sc);
  void
  AddEquipment (std::string const &id, std::shared_ptr<EquipmentBase> ep);
  void
  EnableLogicShifter ();
  void SetScheduleFile(const char* name);
  void SaveSchedules()const;
  void StartEquipment();
  /**
   * Start equipment and scheduling.
   */
  void Go();
  void AllStop();
private:
  std::shared_ptr<GblDataImpl> pimpl;

};

} /* namespace SwitchTiming */

#endif /* SRC_ROOTDATA_H_ */
