/*
 * main.cpp
 *
 *  Created on: Jan 16, 2021
 *      Author: dmounday
 */
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "DS18B20Master.h"
#include <thread>
#include <chrono>

using namespace SwitchTiming;

int main(int argc, char *argv[])
{
  DS18B20Master thermo(60);  // 60 second refresh of values.

  std::vector<std::string> sensors = thermo.GetSensorNames();
  while (true){
      for ( auto s: sensors)
      std::cout << s << ": " << thermo.GetTempC(s) << " C  "
      << thermo.GetTempF(s)<< " F"<< std::endl;

      std::this_thread::sleep_for(std::chrono::seconds(60));
  }

}



