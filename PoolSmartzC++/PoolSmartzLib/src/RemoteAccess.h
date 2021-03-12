/*
 * RemoteAccess.h
 *
 *  Created on: Feb 15, 2021
 *      Author: dmounday
 */

#ifndef SRC_REMOTEACCESS_H_
#define SRC_REMOTEACCESS_H_
#include <memory>

#include "GblData.h"
#include "JsonHandler.h"
namespace SwitchTiming {

class RemoteAccess {
    std::string localIP_;
    short unsigned int localPort_;
    std::string protocol_;
    GblData& gD_;

public:
    RemoteAccess(const pt::ptree& node, GblData& );
    void Start();
};

} /* namespace SwitchTiming */

#endif /* SRC_REMOTEACCESS_H_ */
