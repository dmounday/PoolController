/*
 * MomentarySwitch.h
 *
 *  Created on: Jun 28, 2020
 *      Author: dmounday
 */

#ifndef SRC_MOMENTARYSWITCH_H_
#define SRC_MOMENTARYSWITCH_H_

#include "RelaySwitcher.h"
namespace SwitchTiming {

constexpr std::chrono::nanoseconds MIN_TRIGGER_INTERVAL {std::chrono::nanoseconds(2*1000*1000)};
constexpr std::chrono::nanoseconds MIN_HOLD_INTERVAL {std::chrono::nanoseconds(500*1000*1000)};
///
/// Class to implement the momentary push button to change state of RelaySwitcher base class.
/// A push of the momentary switch changes the associated RelaySwitcher from on to off or off to on.
///  id: ID associated with RelaySwitcher - for log and display purposes.
///  switcher_pin: GPIO pin that drives relay.
///  button_pin: GPIO pin monitored for momentary on.
///  led_pin: If the switch has an pilot LED this specifies the led pin.

class MomentarySwitch: public RelaySwitcher {
public:
	//MomentarySwitch(const std::string& id, unsigned switcher_pin, unsigned button_pin);
	MomentarySwitch(const std::string& id, unsigned switcher_pin, unsigned button_pin,
			unsigned led_pin = 0);
	virtual ~MomentarySwitch();

private:
	GPIO::PushButton     momentary_;
	unsigned button_pin_; 	// GPIO pin for momentary switch
	bool button_pushed_;

};
}
#endif /* SRC_MOMENTARYSWITCH_H_ */
