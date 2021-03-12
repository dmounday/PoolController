/*
 * MainPumpSchedule.cpp
 *
 *  Created on: Feb 13, 2021
 *      Author: dmounday
 */

#include "MainPumpSchedule.h"

namespace SwitchTiming {

MainPumpSchedule::MainPumpSchedule (boost::asio::io_context& ioc,
                  pt::ptree& node,
                  EquipmentPtr eq ):
         EquipSched(ioc, node, eq)
{
    speed_ = node.get("Settings.Speed", 0);
    PLOG(plog::debug)<< "Speed: "<< speed_;
}

} /* namespace SwitchTiming */
