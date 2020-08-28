/*
 * Schedule.cpp
 *
 *  Created on: Jun 29, 2020
 *      Author: dmounday
 */
#include <boost/bind/bind.hpp>
#include "plog/Log.h"
#include "cppgpio.hpp"   // defined make_unique if needed.
#include "Schedule.h"

namespace SwitchTiming {
using namespace boost::posix_time;

Schedule::Schedule(const std::string &sched_file, Equipment &equip) {
	std::ifstream sched_in(sched_file);
	if (sched_in) {
		ParseFile(sched_in, equip);
	} else {
		std::string ex_msg { "Unable to process schedule file: " };
		ex_msg += sched_file;
		throw ScheduleFileException(ex_msg.c_str());
	}

}

bool Schedule::ParseFile(std::ifstream &inf, Equipment &equip) {
	std::string line;
	std::string id;
	std::string start_time;
	std::string stop_time;
	getline(inf, line);
	while (!inf.eof()) {
		size_t s { 0 };
		PLOG(plog::debug) << "Line: " << line;
		if (line[0] != '#' && line.length() > 0) {
			size_t e = line.find_first_of(" \t");
			if (e != std::string::npos) {

				id = line.substr(s, e);
				s = line.find_first_not_of(" \t", e);
				e = line.find_first_of(" \t", s);
				start_time = line.substr(s, e - s);
				bool manual = start_time == "manual";
				if (!manual) {
					if (e != std::string::npos) {
						s = line.find_first_not_of(" \t", e);
						stop_time = line.substr(s);
					} else {
						return false;
					}
				}
				auto eq = equip.find(id);
				if (eq != equip.end()) {

					if (manual) {
						schedules_.push_back( { eq->second, manual });
					} else {
						schedules_.push_back( { eq->second, manual, start_time,
								stop_time });
					}
					PLOG(plog::debug) << "ID:" << id << " name: "
							<< eq->second->Name() << " start: " << start_time
							<< " stop: " << stop_time;
				}
			}
		}
		getline(inf, line);
	}
	PLOG(plog::debug) << "ParseFile complete";
	return true;
}
int Schedule::GetSeconds(std::string hr_min) {
	std::tm t;
	std::istringstream ss(hr_min);
	ss >> std::get_time(&t, "%H:%M");
	if (ss.fail())
		return -1;
	return t.tm_hour * 3600 + t.tm_min * 60;
}

std::chrono::system_clock::duration Schedule::duration_since_midnight() {
	auto now = std::chrono::system_clock::now();
	time_t tnow = std::chrono::system_clock::to_time_t(now);
	tm *date = std::localtime(&tnow);
	date->tm_hour = 0;
	date->tm_min = 0;
	date->tm_sec = 0;
	auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));
	return now - midnight;
}

void Schedule::StopEquipment(const boost::system::error_code &ec,
		EquipSched *eq) {
	eq->relay->SwitchOff();
	eq->CalcStartStopTimes();
	long unsigned after =eq->StartAfter();
	PLOG(plog::debug)<< "Start "<< eq->relay->Name()<< " in " << after;
	eq->btimer->expires_after(std::chrono::seconds(after));
	eq->btimer->async_wait(
			std::bind(&Schedule::StartEquipment, this, std::placeholders::_1,
					eq));

}
void Schedule::StartEquipment(const boost::system::error_code &ec,
		EquipSched *eq) {
	// Start equip id.
	eq->relay->SwitchOn();
	long unsigned after = eq->StopAfter();
	PLOG(plog::debug)<< "Stop "<< eq->relay->Name()<< " in " << after;
	eq->btimer->expires_after(std::chrono::seconds(after));
	eq->btimer->async_wait(
			std::bind(&Schedule::StopEquipment, this, std::placeholders::_1,
					eq));

}

void Schedule::RunSchedule(boost::asio::io_context &io) {
	PLOG(plog::debug) << " ";
	for (auto &eq : schedules_) {
		if (eq.manual)
			continue;
		eq.CalcStartStopTimes();
		eq.btimer = std::make_unique<boost::asio::system_timer>(io); // start timer
		unsigned long after = eq.StartAfter();
		PLOG(plog::debug)<< "Start "<< eq.relay->Name()<< " in " << after;
		eq.btimer->expires_after(std::chrono::seconds(after));
		eq.btimer->async_wait(
				std::bind(&Schedule::StartEquipment, this,
						std::placeholders::_1, &eq));

	}
}

} /* namespace SwitchTiming */
