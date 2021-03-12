/*
 * Switcher.cpp
 *
 *  Created on: Jun 27, 2020
 *      Author: dmounday
 */

#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <boost/optional.hpp>
#include "plog/Log.h"
#include "RelaySwitcher.h"

namespace SwitchTiming {

///
/// RelaySwitcher represents a GPIO that drives a relay.

RelaySwitcher::RelaySwitcher(const std::string& name, unsigned pin, unsigned led_pin):
	EquipmentBase(name),
	pin_ {pin}, pilot_pin_{led_pin}, state_{RelaySwitcher::RelayState::OFF},
	switch_(pin),
	pilot_(led_pin)
  {
    switch_.off();
    PLOG(plog::debug)<< "Relay: "<< name;
  }

RelaySwitcher::~RelaySwitcher() {
	switch_.off();
}

const std::string&
RelaySwitcher::StateStr()const{
	if ( state_== RelayState::ON)
		return ON;
	else if (state_ == RelayState::OFF )
		return OFF;
	return Error;
}

void RelaySwitcher::SwitchOn () {
  if ((state_ != RelayState::ON) && CheckConditions (SwitchRequest::ON)) {
    switch_.on ();
    state_ = RelayState::ON;
    if (pilot_pin_)
      pilot_.on ();
    on_time_ = std::chrono::system_clock::now ();
    PLOG(plog::info) << "Switch ON " << name_;

  }
}

void RelaySwitcher::SwitchOff () {
  if ((state_ != RelayState::OFF) && CheckConditions (SwitchRequest::OFF)) {
    switch_.off ();
    state_ = RelayState::OFF;
    if (pilot_pin_)
      pilot_.off ();
    off_time_ = std::chrono::system_clock::now ();
    PLOG(plog::info) << "Switch OFF " << name_;
  }
  return;
}

void RelaySwitcher::Stop(){
  SwitchOff();
}
void RelaySwitcher::RequiredStates (const Equipment &equipment,
                                    const pt::ptree &node) {
  try {
    auto onConditions = node.get_child_optional ("OnCondition");
    if (onConditions) {
      for (const auto &other : *onConditions) {
        // step thru Equipment names
        PLOG(plog::debug) << "OnCondition: " << other.first <<": "
            << other.second.get_value (other.first);
        if (auto e = equipment.find (other.first); e != equipment.end())
        {
          const std::string onoff = other.second.get_value (other.first);
          RelayState rs {RelayState::ON};
          ForceState fs {ForceState::FORCE_REQUIRED};
          if ( onoff == "require_on"){
            fs = ForceState::ABORT_REQUEST;
          } else if ( onoff == "off"){
            rs = RelayState::OFF;
          }
          std::shared_ptr<RelaySwitcher> rp =
              std::dynamic_pointer_cast<RelaySwitcher>(e->second);
          RequiredState s { rp, SwitchRequest::ON, rs, fs};
              //onoff == "on" ? RelayState::ON : RelayState::OFF,
              //ForceState::FORCE_REQUIRED };
          required_states_.push_back (s);
        } else {
          PLOG(plog::error)<< "OnCondition Equipment label error: "<< other.first;
        }
      }
    } else {
      PLOG(plog::debug)<< "No OnConditions";
    }

    auto offConditions = node.get_child_optional ("OffCondition");
    if (offConditions) {
      for (const auto &other : *offConditions) {
        PLOG(plog::debug) << "OffConditions: " << other.first <<": "
            << other.second.get_value (other.first);
        if (auto e = equipment.find (other.first); e != equipment.end())
        {
          const std::string onoff = other.second.get_value (other.first);
          RelayState rs {RelayState::ON};
                    ForceState fs {ForceState::FORCE_REQUIRED};
                    if ( onoff == "require_on"){
                      fs = ForceState::ABORT_REQUEST;
                    } else if ( onoff == "off"){
                      rs = RelayState::OFF;
                    }
          std::shared_ptr<RelaySwitcher> rp =
              std::dynamic_pointer_cast<RelaySwitcher>(e->second);
          RequiredState s { rp, SwitchRequest::OFF, rs, fs};
              //onoff == "on" ? RelayState::ON : RelayState::OFF,
              //ForceState::FORCE_REQUIRED };
          required_states_.push_back (s);
        } else {
          PLOG(plog::error)<< "OffCondition Equipment label error: "<< other.first;
        }
      }
    } else {
      PLOG(plog::debug)<< "No OffConditions:";
    }
  } catch (pt::ptree_error &e) {
    PLOG(plog::error) << "Required States error: " << e.what ();
    throw;
  } catch (std::exception &e) {
    PLOG(plog::error) << "Unknow exception:" << e.what ();
    throw;
  }
}
///
/// Return true if conditions are met.
bool RelaySwitcher::CheckConditions(SwitchRequest switch_req)
{
	for ( const auto rs: required_states_)
	{
		if ( rs.onOff == switch_req	&& rs.relaySwitch->State() != rs.requiredState ){
			if ( rs.forceState == ForceState::FORCE_REQUIRED ){
				if ( rs.requiredState == RelayState::ON )
					rs.relaySwitch->SwitchOn();
				else
					rs.relaySwitch->SwitchOff();
			} else {
				return false;
			}
		}

	}
	return true;
}

StatusList
RelaySwitcher::GetStatusList() {
	StatusList states;
	states.push_back(std::make_pair("status", StateStr()));
	std::ostringstream time_str;
	std::time_t t_c = std::chrono::system_clock::to_time_t(on_time_);
	time_str << std::put_time(std::localtime(&t_c), "%F %T");
	states.push_back(std::make_pair("time_on", time_str.str() ));

	t_c = std::chrono::system_clock::to_time_t(off_time_);
	std::ostringstream time_off_str;
	time_off_str << std::put_time(std::localtime(&t_c), "%F %T");
	states.push_back(std::make_pair("time_off", time_off_str.str() ));

	return states;
}
} // namespace



