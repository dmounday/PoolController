/*
 * SI7021.cpp
 *
 *  Created on: Jul 11, 2020
 *      Author: dmounday
 */
#include <iostream>
#include "SI7021.h"
#include "plog/Log.h"
//#define DEBUG
#ifdef DEBUG
#define DBG_LOG(x) x
#else
#define DBG_LOG(x)
#endif
namespace GPIO {

const unsigned int SI7021_I2CADDR {0x40};
const uint8_t SI7021_READTEMP {0xF3};  // Read Temp, No Hold Master mode.
const uint8_t SI7021_READHUM {0xF5};   // Read RH, NO Hold Master mode.
const uint8_t SI7021_WRITEREG {0xE6};  // Write RH/T user Register 1
const uint8_t SI7021_READREG {0xE7};   // Read RH/T User Register 1
const uint8_t SI7021_RESET	{0xFE};    // Reset

SI7021::SI7021(const std::string& name, const std::string& interface, boost::asio::io_context& io,
		unsigned int sample_interval):
	EquipmentBase(name),
	ioc_{io},
	i2c(interface, SI7021_I2CADDR),
	last_humidity_{0.0}, last_temp_{0.0}
{
	interval = sample_interval;
	if ( interval > 0){
		s_timer = std::make_unique<boost::asio::steady_timer>(io);
		s_timer->expires_after(std::chrono::seconds(interval));
		s_timer->async_wait(
				std::bind(&SI7021::Sample, this,  std::placeholders::_1));

	}
}

SI7021::~SI7021() {
}
void SI7021::Sample(const boost::system::error_code& ec){
	if ( ec )
		return;
	ReadTemp();
	ReadHumidity();
	DBG_LOG(PLOG(plog::debug) << "Temperature: " << last_temp_ <<
			" C  Humidity: " << last_humidity_);
	s_timer->expires_after(std::chrono::seconds(interval));
	s_timer->async_wait(
			std::bind(&SI7021::Sample, this,  std::placeholders::_1));
}
void SI7021::Reset(){
	uint8_t data {SI7021_RESET};
	i2c.write(&data, 1);
	boost::asio::steady_timer tmr(ioc_, boost::asio::chrono::milliseconds(50));
	tmr.wait();

	data = i2c.regread8(SI7021_READREG);
	if ( data != 0x3a ) {
		PLOG(plog::error)<< "Reset failed: "<< data;
	}
}
float SI7021::ReadTemp(){
	uint8_t data {SI7021_READTEMP};
	i2c.write(&data, 1);
	boost::asio::steady_timer tmr(ioc_, boost::asio::chrono::milliseconds(50));
	tmr.wait();
	uint8_t t[2];
	i2c.read(t, 2);
	float temp = ( (t[0])<<8)|(t[1]&0xfc);
	temp *= 175.72f;
	temp /= 65536.0f;
	temp -= 46.85f;
	last_temp_ = temp;
	return temp;
}

float SI7021::ReadHumidity(){
	uint8_t cmd {SI7021_READHUM};
	i2c.write(&cmd, 1);
	boost::asio::steady_timer tmr(ioc_, boost::asio::chrono::milliseconds(50));
	tmr.wait();
	uint8_t h[2];
	i2c.read(h, 2);
	last_humidity_ = (((h[0]*256 +h[1])* 125.0)/65536.0)-6;
	return last_humidity_;
}

SwitchTiming::StateList
SI7021::GetStateList(){
	SwitchTiming::StateList states;
	states.push_back(std::make_pair("Humidity", std::to_string(last_humidity_)));
	states.push_back(std::make_pair("Temperature", std::to_string(last_temp_)));
	return states;
}
} /* namespace SwitchTiming */

