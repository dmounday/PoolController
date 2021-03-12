/*
 * MainPumpSchedule.h
 *
 *  Created on: Feb 13, 2021
 *      Author: dmounday
 */

#ifndef SRC_MAINPUMPSCHEDULE_H_
#define SRC_MAINPUMPSCHEDULE_H_

#include "EquipSched.h"


namespace SwitchTiming {

class MainPumpSchedule : public EquipSched {
public:
  MainPumpSchedule (boost::asio::io_context& ioc,
                    pt::ptree& node,
                    EquipmentPtr eqp );
private:
  int speed_;     // pump speed setting
};

} /* namespace SwitchTiming */

#endif /* SRC_MAINPUMPSCHEDULE_H_ */
