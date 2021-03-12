/*
 * WirelessHandler.h
 *
 *  Created on: Jul 26, 2020
 *      Author: dmounday
 */

#ifndef SRC_WIRELESSHANDLER_H_
#define SRC_WIRELESSHANDLER_H_
#include <string>
#include <map>
#include "plog/Log.h"
#include "SerialPort.h"
#include "WirelessSensors.h"

namespace SwitchTiming {

using RT = std::map<const int, WirelessSensors*>;
//
// Handles one or more WirelessRT that report on the same serial port.
//
class WirelessHandler: public SerialPort {
	RT rts_;
	void UpdateSensors(const std::string& );
	void on_receive_(const std::string& data)override;
public:
	// port is a tty port
	//
	WirelessHandler(boost::asio::io_context& ioc, const std::string& port, int baud_rate);
	inline void ConfigureRT(const int id, WirelessSensors* rt) {
		rts_[id] = rt;
		//PLOG(plog::debug)<< "RT ID: "<< id;
	}


};

} /* namespace WirelessSerialSensors */

#endif /* SRC_WIRELESSHANDLER_H_ */
