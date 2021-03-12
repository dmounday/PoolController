/*
 * GblDataImpl.cpp
 *
 *  Created on: Feb 16, 2021
 *      Author: dmounday
 */

#include "GblDataImpl.h"

namespace SwitchTiming {

GblDataImpl::GblDataImpl (boost::asio::io_context& ioc):
  ioc_{ioc}
{}
EquipmentPtr
GblDataImpl::Sensor(std::string const& id)const
{
  return sensorDope_->Sensor(id);
}

float GblDataImpl::GetSensorValue(std::string const& id) const{
  auto sm =  std::static_pointer_cast<SensorModule>(Sensor(id));
  return sm->GetSensorValue(id);
}

EquipmentPtr
GblDataImpl::Equip(std::string const& id)const
{
  try {
    return equipment_.at(id);
  } catch (std::out_of_range& e){
      PLOG(plog::error)<< id << ": "<< e.what();
      throw(e);
  }
}
void
GblDataImpl::StartEquipment(){
  for ( auto& e: equipment_){
    // Start all equipment objects
    PLOG(plog::debug)<< "Start: "<< e.first;
    e.second->Start();
  }
}
void
GblDataImpl::StopEquipment(){
  for ( auto& e: equipment_){
    // Start all equipment objects
    PLOG(plog::debug)<< "Stop: "<< e.first;
    e.second->Stop(); // Stop also switches off relays
  }
}
void GblDataImpl::SetScheduleFile(const char* name){
  schedFile_ = name;
}
void GblDataImpl::SaveSchedules() const{
  std::string fn = schedFile_+".rt";
  pt::write_json(fn, runProps_);
}
void
GblDataImpl::Go(){
  StartEquipment();
  PLOG(plog::debug)<< "RunSchedule";
  schedConfig_->RunSchedule();
  PLOG(plog::debug)<< "Start RemoteAccess";
  remoteAccess_->Start();
}
void
GblDataImpl::AllStop(){
  PLOG(plog::info);
  StopEquipment();
  ioc_.stop();

}
} /* namespace SwitchTiming */
