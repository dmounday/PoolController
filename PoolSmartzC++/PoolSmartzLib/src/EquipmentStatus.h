/*
 * EquipmentStatus.h
 *
 *  Created on: Jul 22, 2020
 *      Author: dmounday
 */

#ifndef SRC_EQUIPMENTSTATUS_H_
#define SRC_EQUIPMENTSTATUS_H_

#include <string>
#include <boost/utility/string_view.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "GblData.h"
#include "RelaySwitcher.h"

namespace SwitchTiming {
namespace pt = boost::property_tree;


class EquipmentStatus {

	pt::ptree root;
public:
	EquipmentStatus(GblData const& gD, boost::string_view req);
	std::string operator()();
};

} /* namespace SwitchTiming */

#endif /* SRC_EQUIPMENTSTATUS_H_ */
