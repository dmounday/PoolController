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
#include "WirelessRT.h"

namespace WirelessSerialSensors {

using RT = std::map<const int, WirelessRT*>;
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
	inline void ConfigureRT(const int id, WirelessRT* rt) {
		rts_[id] = rt;
		//PLOG(plog::debug)<< "RT ID: "<< id;
	}


};

} /* namespace WirelessSerialSensors */

#endif /* SRC_WIRELESSHANDLER_H_ */
