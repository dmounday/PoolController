/*
 * EquipSched.cpp
 *
 *  Created on: Feb 10, 2021
 *      Author: dmounday
 */
//#define DEBUG
#ifdef DEBUG
#define DBG_LOG(x) x
#else
#define DBG_LOG(x)
#endif
#include "EquipSched.h"

namespace SwitchTiming {
using namespace boost::posix_time;
namespace bt = boost::posix_time;
EquipSched::EquipSched(boost::asio::io_context& ioc,
                       pt::ptree& node,
                       EquipmentPtr ep):
            ioc_{ioc}, ep_{ep},
            timer_{ioc},manual_{false},
            runNow_{false}, schedProp_{node}
{
  GetStartStop(schedProp_);

}
void EquipSched::GetStartStop(const pt::ptree& node){
  try {
    std::string start = node.get<std::string> ("Start", "manual");
    if (start != "manual") {
      startTime = node.get<std::string> ("Start", "99:99");
      stopTime = node.get<std::string> ("Stop", "99:99");
    } else {
      manual_ = true;
      startTime = "manual";
      stopTime = "manual";
    }
    PLOG(plog::debug)<< ep_->Name() << "Manual:" << manual_ << " Start: "<< startTime << " Stop: "<< stopTime;
  } catch (pt::ptree_error &e) {
    PLOG(plog::error) << "EquipSched file error: " << e.what ();
    throw(e);
  }
}

void EquipSched::Reschedule(const pt::ptree& node){
  GetStartStop( node );
  // Update original runconfig file.
  schedProp_.put("Start", startTime);
  schedProp_.put("Stop", stopTime);
  timer_.cancel();
  if ( RunNow() ){
    Run();
  } else {
    ep_->SwitchOff();
  }
}

void EquipSched::SetSensor(const pt::ptree&){};

void EquipSched::CalcStartStopTimes () {
  if (manual_)
    return;
  runNow_ = false;
  ptime current (second_clock::local_time ());
  time_duration now (current.time_of_day ());     // current time of day
  ptime today (current.date ());
  DBG_LOG(PLOG(plog::debug) << "StartStop time for " << ep_->Name ());
  bt::time_duration start_time_of_day = duration_from_string (startTime);
  bt::time_duration stop_time_of_day =duration_from_string (stopTime);
  try {
    if (stop_time_of_day < start_time_of_day) {
      // runs past midnight
      stop_time_of_day += hours (24);
    }
    if (start_time_of_day < now && stop_time_of_day > now) {
      // past start time but not past stop time. Start now.
      start_date_time = current;
      stop_date_time = today + stop_time_of_day;
      runNow_ = true;
    } else if (start_time_of_day > now) {
      start_date_time = today + start_time_of_day;
      stop_date_time = today + stop_time_of_day;
    } else {
      // start tomorrow
      start_date_time = today + start_time_of_day + hours (24);
      stop_date_time = today + stop_time_of_day + hours (24);

    }
    DBG_LOG(PLOG(plog::debug) << "Running from " << start_date_time << " to " << stop_date_time);
  } catch (std::exception &e) {
    PLOG(plog::error) << "time calc error: " << ep_->Name ();

    std::cout << "Exception: " << e.what () << std::endl;
    throw e;
  }
}

bool EquipSched::RunNow(){
  CalcStartStopTimes();
  return runNow_;
}

long unsigned EquipSched::StopAfter () {
  time_duration after = stop_date_time - (ptime) (second_clock::local_time ());
  return after.total_seconds ();
}
long unsigned EquipSched::StartAfter () {
  time_duration after = start_date_time - (ptime) (second_clock::local_time ());
  return after.total_seconds ();
}
int EquipSched::GetSeconds(std::string hr_min) {
  std::tm t;
  std::istringstream ss(hr_min);
  ss >> std::get_time(&t, "%H:%M");
  if (ss.fail())
    return -1;
  return t.tm_hour * 3600 + t.tm_min * 60;
}

std::chrono::system_clock::duration
EquipSched::duration_since_midnight() {
  auto now = std::chrono::system_clock::now();
  time_t tnow = std::chrono::system_clock::to_time_t(now);
  tm *date = std::localtime(&tnow);
  date->tm_hour = 0;
  date->tm_min = 0;
  date->tm_sec = 0;
  auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));
  return now - midnight;
}

void EquipSched::StopEquipment(const boost::system::error_code &ec) {
  if ( ec == boost::asio::error::operation_aborted)
    return;
  ep_->SwitchOff();
  CalcStartStopTimes();
  long unsigned after = StartAfter();
  PLOG(plog::debug)<< "Start "<< ep_->Name()<< " in " << after;
  timer_.expires_after(std::chrono::seconds(after));
  timer_.async_wait(
      std::bind(&EquipSched::StartEquipment, this, std::placeholders::_1));

}
void EquipSched::StartEquipment(const boost::system::error_code &ec) {
  if ( ec == boost::asio::error::operation_aborted)
    return;
  // Start equip id.
  ep_->SwitchOn();
  long unsigned after = StopAfter();
  PLOG(plog::debug)<< "Stop "<< ep_->Name()<< " in " << after;
  timer_.expires_after(std::chrono::seconds(after));
  timer_.async_wait(
      std::bind(&EquipSched::StopEquipment, this, std::placeholders::_1));

}

void EquipSched::Run(){
  PLOG(plog::debug);
  if (manual_)
    return;
  CalcStartStopTimes();
  unsigned long after = StartAfter();
  PLOG(plog::debug) << "Start "<< ep_->Name() << " in "<< after;
  timer_.expires_after(std::chrono::seconds(after));
  timer_.async_wait(
      std::bind(&EquipSched::StartEquipment, this, std::placeholders::_1));


}
} /* namespace SwitchTiming */
