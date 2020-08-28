/*
 * configuration.h
 *
 *  Created on: Jun 27, 2020
 *      Author: dmounday
 */

#ifndef SRC_CONFIGURATION_H_
#define SRC_CONFIGURATION_H_
#include <iostream>
#include <chrono>
const unsigned int LOGIC_ENABLE 			{23};	// Level shifter enable.
const unsigned int MAIN_PUMP_GPIO 			{17};	// main pump
const unsigned int MAIN_PUMP_TOGGLE_GPIO 	{24};	// momentary push button for pump.
const unsigned int IC40_GPIO 				{22};	// IC40 Salt Cell
const unsigned int IC40_TOGGLE_GPIO			{5};	// momentary push button for IC40
const unsigned int BOOSTER_PUMP_GPIO 		{27};	// Booster pump
const unsigned int BOOSTER_PUMP_TOGGLE_GPIO {16};	//
const unsigned int POOL_LIGHT_GPIO 			{26};	// Pool Light
const unsigned int POOL_LIGHT_TOGGLE_GPIO 	{12};	//
//
const unsigned int MAIN_PUMP_PILOT {6};
const unsigned int IC40_PILOT_LED {19};				// Pilot LED for IC40 push button switch.

const unsigned int POOL_LIGHT_PILOT {13};
const unsigned int BOOSTER_PILOT {21};
//
// SI7021 I2C interface
const std::string I2C_INTERFACE {"/dev/i2c-1"};
const int SI7021_READ_INTERVAL {60};
//
// Wireless RT IDs
const int WATERTEMP_RT {790};
const int TEST_RT {791};
const std::string WIRELESS_SERIAL_PORT {"/dev/ttyAMA0"};
const int BAUD_RATE {115200};




#endif /* SRC_CONFIGURATION_H_ */
