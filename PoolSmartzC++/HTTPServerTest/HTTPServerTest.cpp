/*
 * HTTPServerTest.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: dmounday
 */
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <csignal>
#include "plog/Log.h"
#include "plog/Appenders/ConsoleAppender.h"
#include "HTTPServerLib.h"


class Equipment {
public:
  std::string getStatus {"Somethign status"};
  void changeSomething(){
    PLOG(plog::info)<< "Something changed";
  }
};
//------------------------------------------------------------------------------
class HTTP_handler: public std::enable_shared_from_this<HTTP_handler> {
public:
  HTTP_handler(Equipment& equipment) :
      equipment_ { equipment } {
  }

  std::string PostHandler(beast::string_view path, beast::string_view json) {
    PLOG(plog::debug) << "path: " << path;
    PLOG(plog::debug) << "Json request: " << json;
    return std::string{"something changed."};
  }

  std::string GetHandler(beast::string_view path) {
    result = equipment_.getStatus;
    return result;
  }
  std::string RequestHandler(http::verb method, beast::string_view path,
      beast::string_view body_str) {
    if (method == http::verb::get)
      return GetHandler(path);
    else
      return PostHandler(path, body_str);
  }
  std::shared_ptr<HTTP_handler> getptr() {
    return shared_from_this();
  }
private:
  std::string result;
  Equipment& equipment_;
};

boost::asio::io_context ioc;
bool signal_seen {false};

void terminate_handler(int signal_num){
  if ( ! signal_seen ){
    signal_seen = true;
  }
  ioc.stop();
  exit(signal_num);
}
int main(int argc, char *argv[]) {
  struct sigaction action;
  action.sa_handler = terminate_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGHUP, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  std::cerr << "Pool Smartz Initialization" << std::endl;
  if (argc < 2) {
    std::cerr << "Usage:  [fatal|warning|info|none|error|debug|verbose]\n";
    return 1;
  }

  enum plog::Severity severity { plog::none };
  if (argc == 2) {
    if (!strcmp(argv[1], "none"))
      severity = plog::none;
    else if (!strcmp(argv[1], "fatal"))
      severity = plog::fatal;
    else if (!strcmp(argv[1], "error"))
      severity = plog::error;
    else if (!strcmp(argv[1], "warning"))
      severity = plog::error;
    else if (!strcmp(argv[1], "info"))
      severity = plog::info;
    else if (!strcmp(argv[1], "debug"))
      severity = plog::debug;
    else if (!strcmp(argv[1], "verbose"))
      severity = plog::verbose;
  }
  std::cerr << "PLOG severity " << severity << std::endl;
  static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(severity, &consoleAppender);
  // Initailize Http Server ---------------------------------------------------------------------
  // ToDo: Add these to configuration file.
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("8080"));
  Equipment something;
  auto http_handler = std::make_shared<HTTP_handler>(something);
  auto request_handler = std::bind(&HTTP_handler::RequestHandler, http_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

  // Create and launch a listening port ----------------------------------------------------
  std::make_shared<listener>(ioc, tcp::endpoint{address, port},request_handler)
      ->run();
  ioc.run();
}

