/*
 * EquipmentCommands.cpp
 *
 *  Created on: Jul 28, 2020
 *      Author: dmounday
 */

#include "EquipmentCommands.h"
#include "plog/Log.h"

namespace SwitchTiming {
namespace pt = boost::property_tree;

EquipmentCommands::EquipmentCommands(boost::string_view path, boost::string_view json) {

	if ( path == "/set") {
		pt::ptree root;
		std::string str_it (json); // ugly probably and alloc and copy.
		try {
			std::stringstream ss( str_it );
			pt::read_json( ss, root );
			for ( auto& e : root.get_child("Set")){
				equip_name = e.first;
				command = e.second.data();
				return;  // we only want the first one. Should only be one.
			}
		}  catch (pt::ptree_bad_path &e) {
			PLOG(plog::error)<< "JSON parsing error: " << e.what() << json;
		}
	}
	PLOG(plog::error)<< "path: "<< path;
	throw;
}
bool EquipmentCommands::RunCommand(Equipment equip){
	auto eq = equip.find(equip_name);
	if ( eq != equip.end()){
		if ( command == "on") {
			eq->second->SwitchOn();
			return true;
		} else if (command == "off"){
			eq->second->SwitchOff();
			return true;
		}
	}
	PLOG(plog::debug)<< "Operation failure: " << equip_name << ": "<< command;
	return false;
}
} /* namespace SwitchTiming */
