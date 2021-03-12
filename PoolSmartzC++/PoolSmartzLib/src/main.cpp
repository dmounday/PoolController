/*
 * main.cpp
 *
 *  Created on: Jan 19, 2021
 *      Author: dmounday
 */

#include "plog/Log.h"
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "GblData.h"
#include "ConfigGlobal.h"

namespace st = SwitchTiming;
namespace pt = boost::property_tree;
bool signal_seen {false};
static st::GblData* gData;


void terminate_handler(int signal_num){
  if ( ! signal_seen ){
    signal_seen = true;
    gData->AllStop();
  }
  PLOG(plog::info) << "Terminate on signal "<< signal_num;
  exit(signal_num);
}

void ReadConfigFiles( st::GblData& gD,
                      const char* configFile,
                      const char* runSchedFile,
                      const char* severity,
                      bool consoleLog){

  st::ConfigGlobal cg{gD, configFile, runSchedFile, severity, consoleLog};

}


int main (int argc, char *argv[]) {
  struct sigaction action;
  action.sa_handler = terminate_handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  sigaction (SIGINT, &action, NULL);
  sigaction (SIGHUP, &action, NULL);
  sigaction (SIGTERM, &action, NULL);

  if (argc < 3) {
    std::cerr
        << "Usage: PoolAutomation" <<
        " <equip-config.json> <run-file.json> [fatal|warning|info|none|error|debug|verbose] [console]\n";
    return 1;
  }
  try {
    boost::asio::io_context ioc;
    st::GblData gblData { ioc };
    gData = &gblData;
    bool consoleLog = argc == 5 ? !strcmp (argv[4], "console") : false;
    ReadConfigFiles (gblData, argv[1], argv[2], argc >= 4 ? argv[3] : nullptr,
                     consoleLog);
    gblData.Go ();

    ioc.run ();
  } catch (std::exception &e) {
    std::cerr << "Terminate with exception: " << e.what () << std::endl;
  }
  return 0;
}



