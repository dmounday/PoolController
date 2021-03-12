/*
 * GblData.cpp
 *
 *  Created on: Feb 12, 2021
 *      Author: dmounday
 */

#include "GblData.h"
#include "GblDataImpl.h"

namespace SwitchTiming {


GblData::GblData (boost::asio::io_context &ioc) :
    pimpl { std::make_shared<GblDataImpl> (ioc) } {
}
EquipmentPtr GblData::Sensor (std::string const &id) const {
  return pimpl->Sensor (id);
}

EquipmentPtr GblData::Equip (std::string const &id) const {
  return pimpl->Equip (id);
}
float
GblData::GetSensorValue(std::string const& id) const {
  return pimpl->GetSensorValue(id);
}
Equipment const&
GblData::AllEquipment () const {
  return pimpl->AllEquipment ();
}
boost::asio::io_context&
GblData::IOC () const {
  return pimpl->IOC ();
}
ConfigScheduling&
GblData::Scheduling()const {
  return pimpl->Scheduling();
}
std::shared_ptr<RemoteAccess>
GblData::GetRemoteAccess() const {
  return pimpl->GetRemoteAccess();
}
void GblData::SetLogicShifter (std::shared_ptr<LogicLevelShifter> ls) {
  pimpl->SetLogicShifter (ls);
}
void GblData::SetSensorDope (std::shared_ptr<SensorDope> sd) {
  pimpl->SetSensorDope (sd);
}
void GblData::SetRemoteAccess (std::shared_ptr<RemoteAccess> ra) {
  pimpl->SetRemoteAccess (ra);
}
void GblData::SetSchedConfig (std::shared_ptr<ConfigScheduling> sc) {
  pimpl->SetSchedConfig (sc);
}
void GblData::AddEquipment (std::string const &id,
                            std::shared_ptr<EquipmentBase> ep) {
  pimpl->AddEquipment (id, ep);
}
void GblData::EnableLogicShifter () {
  pimpl->EnableLogicShifter ();
}
pt::ptree& GblData::RunProperties()const{
  return pimpl->RunProperties();
}
void GblData::SetScheduleFile(const char* name){
  pimpl->SetScheduleFile(name);
}
void GblData::SaveSchedules() const{
  pimpl->SaveSchedules();
}
void GblData::Go(){
  pimpl->Go();
}
void GblData::AllStop(){
  pimpl->AllStop();
}

} /* namespace SwitchTiming */
