/*
 * SI7021.h
 *
 *  Created on: Jul 11, 2020
 *      Author: dmounday
 */

#ifndef SRC_SI7021_H_
#define SRC_SI7021_H_
#include <string>
#include "boost/asio.hpp"
#include "cppgpio/i2c.hpp"
#include "EquipmentBase.h"

namespace GPIO {


class SI7021: public SwitchTiming::EquipmentBase {
public:
	// construct SI7021 object.
	SI7021(const std::string& name,
		   const std::string& interface, boost::asio::io_context& io,
		   unsigned int interval=0);
	virtual ~SI7021();
	float ReadTemp();
	float ReadHumidity();
	void Reset();
	void StartSampling(unsigned int interval);
	SwitchTiming::StateList GetStateList();
private:
	boost::asio::io_context& ioc_;
	I2C i2c;
	float last_humidity_;
	float last_temp_;
	unsigned int interval;
	std::unique_ptr<boost::asio::steady_timer> s_timer;
	void Sample(const boost::system::error_code& ec);
};

} /* namespace SwitchTiming */

#endif /* SRC_SI7021_H_ */
