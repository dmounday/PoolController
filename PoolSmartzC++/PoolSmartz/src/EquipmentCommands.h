/*
 * EquipmentCommands.h
 *
 *  Created on: Jul 28, 2020
 *      Author: dmounday
 */

#ifndef SRC_EQUIPMENTCOMMANDS_H_
#define SRC_EQUIPMENTCOMMANDS_H_
#include <string>
#include <boost/utility/string_view.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "RelaySwitcher.h"

namespace SwitchTiming {
//
// Class handles commands from the web interface to change the state
// of equipment.
class EquipmentCommands {
	std::string equip_name;
	std::string command;

public:
	EquipmentCommands(boost::string_view path, boost::string_view json);
	bool RunCommand(Equipment equipment);
};

} /* namespace SwitchTiming */

#endif /* SRC_EQUIPMENTCOMMANDS_H_ */
