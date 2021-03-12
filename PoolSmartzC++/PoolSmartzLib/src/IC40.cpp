/*
 * IC40.cpp
 *
 *  Created on: Feb 16, 2021
 *      Author: dmounday
 */

#include "IC40.h"

namespace SwitchTiming {

IC40::IC40 (pt::ptree& prop_node, GblData& gData):
          EquipSched(gData.IOC(), prop_node, gData.Equip(IC_40))
{
  rate = prop_node.get("Settings.Rate", -1);
  PLOG(plog::debug)<< "Rate: "<< rate;
}

} /* namespace SwitchTiming */
