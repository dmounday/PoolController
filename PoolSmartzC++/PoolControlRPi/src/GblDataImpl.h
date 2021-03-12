/*
 * GblDataImpl.h
 *
 *  Created on: Feb 16, 2021
 *      Author: dmounday
 */

#ifndef SRC_GBLDATAIMPL_H_
#define SRC_GBLDATAIMPL_H_
#include "EquipmentBase.h"
#include "SensorDope.h"

#include "ConfigScheduling.h"
#include "JsonHandler.h"
#include "LogicLevelShifter.h"
#include "RemoteAccess.h"
namespace SwitchTiming {

class GblDataImpl {
public:
  GblDataImpl (boost::asio::io_context& ioc);
  EquipmentPtr Sensor(std::string const& id) const;
  EquipmentPtr Equip(std::string const& id) const;
  inline Equipment const& AllEquipment() const {return equipment_;};
  inline std::shared_ptr<RemoteAccess> GetRemoteAccess() const
      {return remoteAccess_;};
  inline pt::ptree& RunProperties(){return runProps_;};
  inline boost::asio::io_context& IOC() const
  { return ioc_;};
  float GetSensorValue(std::string const& id) const;
  inline void
  SetLogicShifter(std::shared_ptr<LogicLevelShifter> ls){ logicShifter_ = ls;};

  inline void
  SetSensorDope(std::shared_ptr<SensorDope> sd){sensorDope_ = sd;};

  inline void
  SetRemoteAccess( std::shared_ptr<RemoteAccess> ra){remoteAccess_ =ra;};

  inline void
  SetSchedConfig( std::shared_ptr<ConfigScheduling> sc){schedConfig_ = sc;};

  inline ConfigScheduling&
  Scheduling()const {
    return *schedConfig_;
  }
  inline void
  AddEquipment(std::string const& id, std::shared_ptr<EquipmentBase> ep ){
    equipment_[id] = ep;
  }
  inline void
  EnableLogicShifter(){logicShifter_->EnableShifter();};

  void SetScheduleFile(const char* fileName);
  void SaveSchedules() const;
  void Go();
  void AllStop();
private:
  boost::asio::io_context& ioc_;
  std::shared_ptr<SensorDope> sensorDope_;
  std::shared_ptr<RemoteAccess> remoteAccess_;
  std::shared_ptr<ConfigScheduling> schedConfig_;
  std::shared_ptr<LogicLevelShifter> logicShifter_;  // +3.3v -> 5V shifter enable
  pt::ptree runProps_;
  std::string schedFile_;   // run properties file name
  Equipment equipment_;
  void StartEquipment();
  void StopEquipment();
};

} /* namespace SwitchTiming */

#endif /* SRC_GBLDATAIMPL_H_ */
