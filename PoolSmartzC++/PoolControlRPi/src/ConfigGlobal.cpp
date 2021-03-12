/*
 * ConfigGlobal.cpp
 *
 *  Created on: Feb 15, 2021
 *      Author: dmounday
 */

#include "ConfigGlobal.h"

namespace SwitchTiming {

ConfigGlobal::ConfigGlobal (GblData &gData, const char *configFile,
                            const char *runSchedFile, const char *severity,
                            bool consoleLog) :
    gD_ { gData } {
  try {
    pt::ptree configProps;
    pt::read_json (configFile, configProps); // parse config_file as json
    //plogConfig.load( configProps.get_child("Logging"), severity);
    PlogConfig (configProps.get_child ("Logging"), severity, consoleLog);
    PLOG(plog::info) << "Program Start ---";
    EquipConfig (configProps);
  } catch (pt::json_parser_error &e) {
    std::cerr << "Parsing error: " << configFile << "': " << e.what ();
    exit (99);
  }
  gD_.SetScheduleFile (runSchedFile);
  std::string runFile{runSchedFile};  //Check running file present
  if ( RunConfig(runFile + ".rt")){
    PLOG(plog::info)<< "Running from: "<< runFile+".rt";
    return;
  } else {
    // Missing or corrupt runtime file. Use Original sched file.
    if ( RunConfig( std::string{runSchedFile})){
      PLOG(plog::info)<< "Running from: "<< runSchedFile;
      return;
    }
  }
  PLOG(plog::error) << "Corrupt or missing " << runSchedFile << std::endl;
  exit (99);

}

void
ConfigGlobal::StateConfig(const pt::ptree& required){
  PLOG(plog::debug);
  try {
      for ( const auto& r: required){
        PLOG(plog::debug)<< "RequiredStates: "<< r.first;
        auto e = gD_.Equip(r.first);
        auto rs = std::dynamic_pointer_cast<RelaySwitcher>(e);
        rs->RequiredStates(gD_.AllEquipment(), r.second);
      }
    } catch (pt::ptree_error &e) {
      std::cerr << "RequiredState config error: " << e.what () << std::endl;
      throw;
    } catch ( std::range_error& e){
      PLOG(plog::error)<< "No equipment found:" << e.what();
    }
}
/**
   * Construct MomentarySwitch or RelaySwitcher with name and property sub-tree
   * such as:
   *
   "Relays": {
    "MainPump":
        {
        "RelayGPIO": "17",
        "SwitchGPIO": "24",   <- optional push button
        "LEDGPIO": 6
        },
**/
void ConfigGlobal::RelayConfig (const pt::ptree &relays) {
  PLOG(plog::debug);
  try {
    for (const auto &relay : relays) {
      const auto &r = relay.second;
      unsigned relayGPIO = r.get<unsigned> ("RelayGPIO", 0);
      unsigned pilotGPIO = r.get<unsigned> ("LEDGPIO", 0);
      unsigned pushButton = r.get<unsigned> ("SwitchGPIO", 0);
      if (pushButton == 0) {
        auto switcher = std::make_shared<RelaySwitcher> (relay.first, relayGPIO,
                                                         pilotGPIO);
        gD_.AddEquipment (relay.first,
                          std::dynamic_pointer_cast<EquipmentBase> (switcher));
      } else {
        auto switcher = std::make_shared<MomentarySwitch> (relay.first,
                                                           relayGPIO,
                                                           pushButton,
                                                           pilotGPIO);
        gD_.AddEquipment (relay.first,
                          std::dynamic_pointer_cast<EquipmentBase> (switcher));
      }
    }
  } catch (pt::ptree_error &e) {
    std::cerr << "Relay config error: " << e.what () << std::endl;
    throw;
  } catch (std::range_error &e) {
    PLOG(plog::error) << "Unknow equipment switcher:" << e.what ();
  }
}

void ConfigGlobal::EquipConfig (const pt::ptree &tree) {

  try {
    gD_.SetLogicShifter (
        std::make_shared<LogicLevelShifter> (tree.get_child (LEVELSHIFTER)));

    auto sdp = std::make_shared<SensorDope> (gD_, tree.get_child (SENSORS));
    gD_.SetSensorDope (sdp);

    auto const &relays = tree.get_child (RELAYS);
    RelayConfig (relays);
    auto const &reqStates = tree.get_child (REQ_STATES);
    StateConfig (reqStates);
    // HTTP remote access
    auto const &remote = tree.get_child (REMOTE_ACCESS);
    gD_.SetRemoteAccess (std::make_shared<RemoteAccess> (remote, gD_));
  } catch (pt::ptree_error &e) {
    PLOG(plog::error) << "Config File error: " << e.what ();
    throw;
  }
  gD_.EnableLogicShifter ();
}

bool ConfigGlobal::RunConfig (const std::string& schedFile) {
  PLOG(plog::debug) << "RunConfig";
  try {
    pt::read_json(schedFile.c_str(), gD_.RunProperties());
} catch (pt::json_parser_error &e) {
  std::cerr << "Parsing error: " << schedFile << "': " << e.what ();
  return false;
} catch (pt::ptree_error &e) {
  std::cerr << "Schedule/Run File error: " << e.what () << std::endl;
  return false;
}
  pt::read_json (schedFile.c_str(), gD_.RunProperties ());
  auto& sched = gD_.RunProperties ().get_child (SCHEDULE);
  gD_.SetSchedConfig (std::make_shared<ConfigScheduling> (sched, gD_));
  return true;
}


} /* namespace SwitchTiming */
