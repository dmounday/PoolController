/*
 * WirelessHandler.cpp
 *
 *  Created on: Jul 26, 2020
 *      Author: dmounday
 */

#include "WirelessHandler.h"

//#define DEBUG
#ifdef DEBUG
#define DBG_LOG(x) x
#else
#define DBG_LOG(x)
#endif
namespace SwitchTiming {

WirelessHandler::WirelessHandler(boost::asio::io_context& io, const std::string& port, int baud_rate):
	SerialPort(io)
{
	if ( !start(port, baud_rate)) {
		PLOG(plog::error)<< "Unable to start serial port "<< port;
	}
}
void
WirelessHandler::on_receive_(const std::string& data){
	return UpdateSensors(data);
}

void WirelessHandler::UpdateSensors(const std::string &data) {
	DBG_LOG(PLOG(plog::debug) << data);
	std::size_t s = data.find('[');
	if (s != std::string::npos) {
		if (isdigit(data[s + 1])) {
			std::size_t end_i;
			++s;
			int id = std::stoi(data.substr(s), &end_i);
			DBG_LOG(PLOG(plog::debug) << "id: " << id);
			s += end_i;
			s = data.find_first_not_of("] ", s);
			if (s != std::string::npos) {
				rts_[id]->Update(data.substr(s));
				return;
			}
		}
	}
	PLOG(plog::info) << "Not Sensor data: " << data;
}
} /* namespace WirelessSerialSensors */
