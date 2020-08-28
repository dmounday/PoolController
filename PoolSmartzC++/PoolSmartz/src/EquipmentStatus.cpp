/*
 * EquipmentStatus.cpp
 *
 *  Created on: Jul 22, 2020
 *      Author: dmounday
 */

#include "EquipmentStatus.h"
#include "plog/Log.h"
namespace SwitchTiming {


EquipmentStatus::EquipmentStatus(Equipment& equip, boost::string_view req) {

	PLOG(plog::debug)<< req;
	if ( req.starts_with("/status")){
		if ( req.find(req, '?') == 7){ // found "status?"
			std::string id {req.substr(7)};
			auto eq = equip.find(id);
			if ( eq != equip.end()){
					pt::ptree unit;  //  contains units state
					StateList state_list = eq->second->GetStateList();
					for ( auto const& state: state_list){
						unit.put( state.first, state.second);
					}
					root.add_child(id, unit);
			}
		} else {
			// return system status
			for (auto const& [id, ptr]: equip){
				pt::ptree unit;  //  contains units state
				StateList state_list = ptr->GetStateList();
				for ( auto const& state: state_list){
					unit.put( state.first, state.second);
				}
				root.add_child(id, unit);
			}
		}
		return;
	}
	PLOG(plog::error) << "Unknown target: " << req;

}
std::string
EquipmentStatus::JSON_Results(){

	std::ostringstream os;
	pt::write_json(os, root);
	return os.str();
}

} /* namespace SwitchTiming */
