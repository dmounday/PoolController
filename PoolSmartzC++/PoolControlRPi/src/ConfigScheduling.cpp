/*
 * ConfigScheduling.cpp
 *
 *  Created on: Feb 10, 2021
 *      Author: dmounday
 */

#include "ConfigScheduling.h"
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

namespace SwitchTiming {

ConfigScheduling::ConfigScheduling (pt::ptree& sched_tree,
                                    GblData& gData ): gD_{gData} {
  try {
    EquipmentPtr mp_eq = gData.Equip(MAIN_PUMP);
    auto& mp = sched_tree.get_child(MAIN_PUMP);
    sched_[MAIN_PUMP] =
        std::make_unique<MainPumpSchedule>(gData.IOC(), mp, mp_eq);

    sched_[IC_40] = std::make_unique<IC40>(sched_tree.get_child(IC_40), gData);

    auto& hpNode = sched_tree.get_child(HEAT_PUMP);
    sched_[HEAT_PUMP] =
        std::make_unique<HeatPumpSchedule>(hpNode, gData);

    sched_[BOOSTER] =
        std::make_unique<EquipSched>(gData.IOC(),
        sched_tree.get_child(BOOSTER), gData.Equip(BOOSTER));

    sched_[LIGHTING] =
        std::make_unique<EquipSched>(gData.IOC(),
        sched_tree.get_child(LIGHTING), gData.Equip(LIGHTING));
  } catch (std::out_of_range& e ){
    PLOG(plog::error)<< "Equipment label error: "<< e.what();
    throw;
  } catch (pt::ptree_error &e) {
    PLOG(plog::error) << "Schedule file error: " << e.what ();
    throw;
  }

}

void ConfigScheduling::RunSchedule() {
  PLOG(plog::debug);
  for (auto &eq : sched_) {
    eq.second->Run();

  }
}
std::string
ConfigScheduling::GetSchedules(){
  std::ostringstream os;
  pt::write_json(os, gD_.RunProperties());
  return os.str();
}

std::string ConfigScheduling::SetSchedules (boost::string_view json) {
  // parse json into property tree
  pt::ptree tree;
  boost::iostreams::stream<boost::iostreams::basic_array_source<char>>
        stream(json.begin(), json.end());
  //const std::string str_it (json); // ugly probably and alloc and copy.
  try {
    pt::read_json (stream, tree);
    for( const auto& eq: tree){
      auto const& name = eq.first;
      sched_[name]->Reschedule(eq.second);
    }
    gD_.SaveSchedules();
    return std::string ("Success");
  } catch (pt::ptree_bad_path &e) {
    PLOG(plog::error) << "Set Schedule msg parsing error: " << e.what ()
        << json;
  } catch (std::range_error &e) {
    PLOG(plog::error) << "Equipment name error in SetSched.";
  }
  return std::string ("Failure");
}
std::string
ConfigScheduling::SetSensors(boost::string_view json){
  pt::ptree sensor_prop;
  boost::iostreams::stream<boost::iostreams::basic_array_source<char>>
        stream(json.begin(), json.end());
  try {
    pt::read_json (stream, sensor_prop);
    for( const auto& eq: sensor_prop){
      auto const& name = eq.first;
      sched_[name]->SetSensor(eq.second);
    }
    gD_.SaveSchedules();
    return std::string ("Success");
  } catch (pt::ptree_bad_path &e) {
    PLOG(plog::error) << "Set Set sensors msg parsing error: " << e.what ()
        << json;
  } catch (std::range_error &e) {
    PLOG(plog::error) << "Equipment name error in setSensors.";
  }
  return std::string("Failure");
}

} /* namespace SwitchTiming */
