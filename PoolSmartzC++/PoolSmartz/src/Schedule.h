/*
 * Schedule.h
 *
 *  Created on: Jun 29, 2020
 *      Author: dmounday
 */

#ifndef SRC_SCHEDULE_H_
#define SRC_SCHEDULE_H_
#include <string>
#include <chrono>
#include <vector>
#include <boost/asio.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include "RelaySwitcher.h"

namespace SwitchTiming {
using namespace boost::posix_time;

class ScheduleFileException: std::exception {
public:
	const char* msg;
	ScheduleFileException(const char* message):msg{message}
	{}
};

class Schedule {
public:
	struct EquipSched {
		EquipmentBase *relay;
		bool manual;
		boost::posix_time::time_duration start_time_of_day;
		boost::posix_time::time_duration stop_time_of_day;
		boost::posix_time::ptime start_date_time;
		boost::posix_time::ptime stop_date_time;
		std::unique_ptr<boost::asio::system_timer> btimer;
		//boost::asio::system_timer t_start;
		//boost::asio::system_timer t_stop;

		EquipSched(EquipmentBase *eq, bool man, std::string start,
				std::string stop) {
			relay = eq;
			manual = man;
			start_time_of_day = duration_from_string(start);
			stop_time_of_day = duration_from_string(stop);
		}
		EquipSched(EquipmentBase *eq, bool man):
			relay{eq}, manual{man}
			{}

		void CalcStartStopTimes() {
			if (manual)
				return;

			ptime current(second_clock::local_time());
			time_duration now(current.time_of_day());     // current time of day
			ptime today(current.date());
			PLOG(plog::debug) << "StartStop time for " << relay->Name();
			try {
				if (stop_time_of_day < start_time_of_day) {
					// runs past midnight
					stop_time_of_day += hours(24);
				}
				if (start_time_of_day < now && stop_time_of_day > now) {
					// past start time but not past stop time. Start now.
					start_date_time = current + seconds(5);
					stop_date_time = today + stop_time_of_day;
				} else if (start_time_of_day > now) {
					start_date_time = today + start_time_of_day;
					stop_date_time = today + stop_time_of_day;
				} else {
					// start tomorrow
					start_date_time = today + start_time_of_day + hours(24);
					stop_date_time = today + stop_time_of_day + hours(24);

				}
				std::cout << "Running from " << start_date_time << " to "
						<< stop_date_time << std::endl;
			} catch (std::exception &e) {
				PLOG(plog::error)<< "time calc error: "<< relay->Name();

				std::cout << "Exception: " << e.what() << std::endl;
				throw e;
			}
		}
		long unsigned StopAfter()
		{
			time_duration after = stop_date_time - (ptime)(second_clock::local_time());
			return after.total_seconds();
		}
		long unsigned StartAfter()
		{
			time_duration after = start_date_time - (ptime)(second_clock::local_time());
			return after.total_seconds();
		}
	};
	Schedule(const std::string& sched_file, Equipment& );

	void RunSchedule(boost::asio::io_context& io);

private:
	bool ParseFile(std::ifstream&, Equipment& );
	bool SetScheduleEntry(std::string id, std::string start_time, std::string stop_time);
	int GetSeconds(std::string hr_min);
	std::chrono::system_clock::duration duration_since_midnight();
	bool CalcStartStopTimes(EquipSched& eq);
	void StartEquipment(const boost::system::error_code &ec, EquipSched*);
	void StopEquipment(const boost::system::error_code &ec, EquipSched*);
	std::vector<EquipSched> schedules_;

};

} /* namespace SwitchTiming */

#endif /* SRC_SCHEDULE_H_ */
