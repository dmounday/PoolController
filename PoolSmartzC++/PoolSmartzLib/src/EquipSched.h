/*
 * EquipSched.h
 *
 *  Created on: Feb 10, 2021
 *      Author: dmounday
 */

#ifndef EQUIPSCHED_H_
#define EQUIPSCHED_H_
#include <string>
#include <chrono>
#include <vector>
#include <exception>
#include <string>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <map>
#include "plog/Log.h"
#include "RelaySwitcher.h"

namespace SwitchTiming {
namespace pt = boost::property_tree;


class ScheduleFileException: std::exception {
public:
  const char* msg;
  ScheduleFileException(const char* message):msg{message}
  {}
};

class EquipSched {
public:

    EquipSched(boost::asio::io_context& ioc,
                           pt::ptree& node,
                           EquipmentPtr ep);
    //EquipSched(boost::asio::io_context& ioc,
    //           bool manual,
    //           EquipmentPtr ep);
    virtual ~EquipSched(){};
    void GetStartStop(const pt::ptree& node);
    void Reschedule(const pt::ptree& node);
    virtual void SetSensor(const pt::ptree& node);
    virtual void Run();

    void CalcStartStopTimes();
    inline bool Maunal()const {return manual_;};
    bool RunNow();
    inline RelayPtr Equip()const {return std::static_pointer_cast<RelaySwitcher>(ep_);};
private:
    boost::asio::io_context& ioc_;
    EquipmentPtr ep_;
    boost::asio::system_timer timer_;
    bool manual_;
    bool runNow_;
    pt::ptree& schedProp_;
    std::string stopTime;
    std::string startTime;

    boost::posix_time::ptime start_date_time;
    boost::posix_time::ptime stop_date_time;

    //boost::asio::system_timer t_start;
    //boost::asio::system_timer t_stop;
    int GetSeconds(std::string hr_min);
    std::chrono::system_clock::duration duration_since_midnight();
    void StopEquipment(const boost::system::error_code &ec);
    void StartEquipment(const boost::system::error_code &ec);
    long unsigned StopAfter();
    long unsigned StartAfter();
};

} /* namespace SwitchTiming */

#endif /* EQUIPSCHED_H_ */
