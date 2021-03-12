/*
 * Switcher.h
 *
 *  Created on: Jun 27, 2020
 *      Author: dmounday
 */

#ifndef SRC_RELAYSWITCHER_H_
#define SRC_RELAYSWITCHER_H_
#include <chrono>
#include <memory>

#include <boost/property_tree/ptree.hpp>
#include <cppgpio.hpp>

#include "EquipmentBase.h"

namespace SwitchTiming {
namespace pt = boost::property_tree;

inline const std::string ON {"ON"};
inline const std::string OFF {"OFF"};
inline const std::string Error{"Error"};

class RelaySwitcher;
using RelayPtr = std::shared_ptr<RelaySwitcher>;

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
  /// SwitchRequest on/off from OnConditions/OffCondictions property.
  ///requiredState  The required state and ForceState of the relay.
  ///  property: on/off: SwitchRequest ON/OFF and ForceRquired
  ///  Property: require_on: SwitchRequest ON and ABORT_REQUEST
	struct RequiredState {
		std::shared_ptr<RelaySwitcher> 			relaySwitch;	// depends on condition of this RelaySwitcher.
		SwitchRequest				onOff;			// requested action
		RelayState        	requiredState;	// must be in this state to switch.
		ForceState					forceState;		// force relay to required state.
	};

	/// Relay switch and GPIO control pin. led_pin specifies the control pin
	/// for the LED pilot light.
	///
	RelaySwitcher(const std::string& name, unsigned pin, unsigned led_pin = 0);
	virtual ~RelaySwitcher();
	void SwitchOn()override final;
	void SwitchOff()override final;
	void Stop() override;
	void RequiredStates(const Equipment&, const pt::ptree& );
	inline RelayState State() const {return state_;}
	const std::string& StateStr() const;
	inline std::chrono::system_clock::time_point OnTime()const {return on_time_;}
	inline std::chrono::system_clock::time_point OffTime()const {return off_time_;}
	virtual StatusList GetStatusList();
private:
	bool CheckConditions(SwitchRequest );
	unsigned pin_;			// GPIO pin for equipment control
	unsigned pilot_pin_;		// GPIO pin for optional pilot LED
	enum RelayState state_;
	GPIO::DigitalOut switch_; //
	GPIO::DigitalOut pilot_;	// optional pilot light
	std::vector<RequiredState> required_states_;

	std::chrono::system_clock::time_point on_time_;
	std::chrono::system_clock::time_point off_time_;

};

} // namespace
#endif /* SRC_RELAYSWITCHER_H_ */
