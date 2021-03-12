/*
 * EquipmentBase.h
 *
 *  Created on: Jul 23, 2020
 *      Author: dmounday
 */

#ifndef SRC_EQUIPMENTBASE_H_
#define SRC_EQUIPMENTBASE_H_
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "plog/Log.h"

namespace SwitchTiming {
// Prop tree property names.
// The following inline globals require C++17 to avoid duplicate definitions.
inline std::string const MAIN_PUMP{"MainPump"};
inline std::string const IC_40{ "IC40"};
inline std::string const BOOSTER{"Booster"};
inline std::string const LIGHTING{"Lighting"};
inline std::string const HEAT_PUMP{"HeatPump"};
inline std::string const SETTINGS{"Settings"};

class EquipmentBase;
//
// Base class for all equipment and sensors.
// Includes pumps, SWG, Heaters, sensors, etc.

using StatusList= std::vector<std::pair<std::string, std::string>>;
using Equipment = std::map<const std::string, std::shared_ptr<EquipmentBase>>;
using EquipmentPtr = std::shared_ptr<EquipmentBase>;

class EquipmentBase {
protected:
	std::string name_;
public:
	EquipmentBase(const std::string name);
	virtual ~EquipmentBase(){};
	inline const std::string& Name(){return name_;}
	virtual void SwitchOff() {};
	virtual void SwitchOn() {};
	/**
	 * Override to stop some activity when shutdown.
	 */
	virtual void Stop() {PLOG(plog::debug);};
	virtual void Start() {PLOG(plog::debug);};
	// Each derived class determines the actual list of
	// statuses returned.
	virtual StatusList GetStatusList() = 0;
};
using EquipmentPtr = std::shared_ptr<EquipmentBase>;
} /* namespace SwitchTiming */

#endif /* SRC_EQUIPMENTBASE_H_ */
