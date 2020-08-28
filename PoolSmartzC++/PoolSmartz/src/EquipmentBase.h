/*
 * EquipmentBase.h
 *
 *  Created on: Jul 23, 2020
 *      Author: dmounday
 */

#ifndef SRC_EQUIPMENTBASE_H_
#define SRC_EQUIPMENTBASE_H_
#include <vector>
#include <string>

namespace SwitchTiming {
//
// Base class for all equipment and sensors.
// Includes pumps, SWG, Heaters, sensors, etc.

using StateList= std::vector<std::pair<std::string, std::string>>;

class EquipmentBase {
protected:
	std::string name_;
public:
	EquipmentBase(const std::string name);
	virtual ~EquipmentBase(){};
	inline const std::string& Name(){return name_;}
	virtual void SwitchOff() {};
	virtual void SwitchOn() {};
	// Each derived class determines the actual list of
	// statuses returned.
	virtual StateList GetStateList() = 0;
};

} /* namespace SwitchTiming */

#endif /* SRC_EQUIPMENTBASE_H_ */
