/*
 * LogicLevelShifter.cpp
 *
 *  Created on: Feb 8, 2021
 *      Author: dmounday
 */


#include "LogicLevelShifter.h"

namespace SwitchTiming {
namespace pt = boost::property_tree;

LogicLevelShifter::LogicLevelShifter (const pt::ptree& node):
  GPIO::DigitalOut(node.get<unsigned int>("GPIO"))
{
  PLOG(plog::debug)<< node.get<unsigned int>("GPIO");
}
LogicLevelShifter::LogicLevelShifter(unsigned int gpio):
  GPIO::DigitalOut(gpio)
{}
LogicLevelShifter::~LogicLevelShifter(){
  off();
}
void LogicLevelShifter::EnableShifter(){
  PLOG(plog::debug);
  on();
}
void LogicLevelShifter::DisableShifter()
{
  PLOG(plog::debug);
  off();
}
} /* namespace SwitchTiming */
