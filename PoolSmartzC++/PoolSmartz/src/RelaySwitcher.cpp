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
#include "plog/Log.h"
#include "RelaySwitcher.h"

namespace SwitchTiming {
const std::string ON {"ON"};
const std::string OFF {"OFF"};
const std::string Error{"Error"};
///
/// RelaySwitcher represents a GPIO that drives a relay.

RelaySwitcher::RelaySwitcher(const std::string& name, unsigned pin, unsigned led_pin):
	EquipmentBase(name),
	pin_ {pin}, pilot_pin_{led_pin}, state_{RelaySwitcher::RelayState::OFF},
	switch_(pin),
	pilot_(led_pin),
	required_states_ {nullptr}
{}

RelaySwitcher::~RelaySwitcher() {
	switch_.off();
}

const std::string&
RelaySwitcher::StateStr(){
	if ( state_== RelayState::ON)
		return ON;
	else if (state_ == RelayState::OFF )
		return OFF;
	return Error;
}
void RelaySwitcher::SwitchOn(){
	CheckConditions( SwitchRequest::ON);
	if ( state_ != RelayState::ON){
		switch_.on();
		state_ = RelayState::ON;
		if ( pilot_pin_ )
			pilot_.on();
		on_time_ = std::chrono::system_clock::now();
		PLOG(plog::info)<< "Switch ON " << name_;
	}
}
void RelaySwitcher::SwitchOff(){
	CheckConditions( SwitchRequest::OFF);
	switch_.off();
	state_ = RelayState::OFF;
	if ( pilot_pin_)
		pilot_.off();
	off_time_ = std::chrono::system_clock::now();
	PLOG(plog::info)<< "Switch OFF " << name_;
	return;
}
///
/// Return true if conditions are met.
bool RelaySwitcher::CheckConditions(SwitchRequest switch_req)
{
	if (required_states_ == nullptr)
		return true;
	for ( const auto rs: *required_states_)
	{
		if ( rs.onOff == switch_req	&& rs.relaySwitch.State() != rs.requiredState ){
			if ( rs.forceState == ForceState::FORCE_REQUIRED ){
				if ( rs.requiredState == RelayState::ON )
					rs.relaySwitch.SwitchOn();
				else
					rs.relaySwitch.SwitchOff();
			} else {
				return false;
			}
		}

	}
	return true;
}

StateList
RelaySwitcher::GetStateList(){
	StateList states;
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



