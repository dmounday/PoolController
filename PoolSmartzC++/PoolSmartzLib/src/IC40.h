/*
 * IC40.h
 *
 *  Created on: Feb 16, 2021
 *      Author: dmounday
 */

#ifndef SRC_IC40_H_
#define SRC_IC40_H_
#include "GblData.h"
#include "EquipSched.h"

namespace SwitchTiming {

class IC40 : public EquipSched {
public:
  IC40 (pt::ptree& prop_node, GblData& gData);
private:
  int rate;       // % rate of generation
};

} /* namespace SwitchTiming */

#endif /* SRC_IC40_H_ */
