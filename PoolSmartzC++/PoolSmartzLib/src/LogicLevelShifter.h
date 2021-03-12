/*
 * LogicLevelShifter.h
 *
 *  Created on: Feb 8, 2021
 *      Author: dmounday
 */

#ifndef SRC_LOGICLEVELSHIFTER_H_
#define SRC_LOGICLEVELSHIFTER_H_
#include "plog/Log.h"

#include <boost/property_tree/ptree.hpp>
#include <cppgpio/output.hpp>

namespace SwitchTiming {
namespace pt = boost::property_tree;
class LogicLevelShifter : public GPIO::DigitalOut {
public:
  LogicLevelShifter (const pt::ptree& node);
  LogicLevelShifter(unsigned int gpio);
  virtual ~LogicLevelShifter();
  void EnableShifter();
  void DisableShifter();
};

} /* namespace SwitchTiming */

#endif /* SRC_LOGICLEVELSHIFTER_H_ */
