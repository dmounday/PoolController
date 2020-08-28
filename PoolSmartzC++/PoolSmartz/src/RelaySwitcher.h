/*
 * Switcher.h
 *
 *  Created on: Jun 27, 2020
 *      Author: dmounday
 */

#ifndef SRC_RELAYSWITCHER_H_
#define SRC_RELAYSWITCHER_H_
#include <chrono>
#include <cppgpio.hpp>

#include "EquipmentBase.h"

namespace SwitchTiming {

class RelaySwitcher: public EquipmentBase {
public:
	enum class RelayState {
		ON,
		OFF,
		ERROR
	};
	enum class SwitchRequest {
		ON,
		OFF
	};
	enum class ForceState {
		FORCE_REQUIRED,
		ABORT_REQUEST
	};
	/// Identifies a set of RelaySwither states that must exist before a change of
	/// relay state is allowed. For example a request to set IC40 on requires that mainPump is currently on.
	/// Required state of RelaySwitchers before a switch change is made. If foreceState is set
	/// to FORCE_REQUIRED the associated RelaySwitcher is changed.
	///
	struct RequiredState {
		RelaySwitcher& 				relaySwitch;	// depends on condition of this RelaySwitcher.
		SwitchRequest				onOff;			// requested action
		RelaySwitcher::RelayState	requiredState;	// must be in this state to switch.
		ForceState					forceState;		// change relay to required state.
	};

	/// Relay switch and GPIO control pin. led_pin specifies the control pin
	/// for the LED pilot light.
	RelaySwitcher(const std::string& name, unsigned pin, unsigned led_pin = 0);
	virtual ~RelaySwitcher();
	void SwitchOn();
	void SwitchOff();

	inline RelayState State(){return state_;}
	const std::string& StateStr();
	inline std::chrono::system_clock::time_point OnTime(){return on_time_;}
	inline std::chrono::system_clock::time_point OffTime(){return off_time_;}
	inline void SetConditions(std::vector<RequiredState>* states){ required_states_ = states;};
	virtual StateList GetStateList();
private:
	bool CheckConditions(SwitchRequest );
	unsigned pin_;			// GPIO pin for equipment control
	unsigned pilot_pin_;		// GPIO pin for optional pilot LED
	enum RelayState state_;
	GPIO::DigitalOut switch_; //
	GPIO::DigitalOut pilot_;	// optional pilot light
	std::vector<RequiredState>* required_states_;

	std::chrono::system_clock::time_point on_time_;
	std::chrono::system_clock::time_point off_time_;

};

using Equipment = std::map<const std::string, SwitchTiming::EquipmentBase*>;

} // namespace
#endif /* SRC_RELAYSWITCHER_H_ */
