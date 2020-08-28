
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
//#include <boost/utility/string_view.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>

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
#include "boost/asio.hpp"

#include "configuration.h"
#include "MomentarySwitch.h"
#include "Schedule.h"
#include "EquipmentStatus.h"
#include "EquipmentCommands.h"

#include "SI7021.h"
#include "WaterTemperatureRT.h"
#include "WirelessHandler.h"
//--------------------------------------------------------------------------------------
// HTTP server adapted from Boost/Beast for PoolSmartz web control.
//    Thanks to Vinnie.
// Hacked in a callback to request handler to process the equipment commands and
// status requests.
//
// Don
// 7-21-2020
//
//------------------------------------------------------------------------------
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, asynchronous
//
//------------------------------------------------------------------------------



namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using HandlerFunction = std::function<std::string(http::verb, beast::string_view, beast::string_view)>;

// Return a reasonable mime type based on the extension of a file.
beast::string_view
mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}
#if 0
// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
    beast::string_view base,
    beast::string_view path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}
#endif
// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
    class Body, class Allocator,
    class Send>
void handle_request(HandlerFunction handler,
		http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send) {
	PLOG(plog::debug);
	// Returns a bad request response
	auto const bad_request = [&req](beast::string_view why) {
		http::response<http::string_body> res { http::status::bad_request,
				req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = std::string(why);
		res.prepare_payload();
		return res;
	};

	// Make sure we can handle the method
	if (req.method() != http::verb::get && req.method() != http::verb::post)
		return send(bad_request("Unknown HTTP-method"));

	// Request path must be absolute and not contain "..".
	if (req.target().empty() || req.target()[0] != '/'
			|| req.target().find("..") != beast::string_view::npos)
		return send(bad_request("Illegal request-target"));
	beast::string_view req_body = req.body();
	std::string response_body = handler(req.method(), req.target(), req_body);
	PLOG(plog::debug) << "Response: " << response_body;
	auto const size = response_body.length();
	// Respond to GET request
	http::response<http::string_body> res { std::piecewise_construct,
			std::make_tuple(std::move(response_body)), std::make_tuple(
					http::status::ok, req.version()) };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, mime_type(".json"));
	res.content_length(size);
	res.keep_alive(req.keep_alive());
	return send(std::move(res));

}

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda
    {
        session& self_;

        explicit
        send_lambda(session& self)
            : self_(self)
        {
        }

        template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<
                http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(
                self_.stream_,
                *sp,
                beast::bind_front_handler(
                    &session::on_write,
                    self_.shared_from_this(),
                    sp->need_eof()));
        }
    };

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    HandlerFunction handler_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_lambda lambda_;

public:
    // Take ownership of the stream
    session(tcp::socket&& socket, HandlerFunction handler)
        : stream_(std::move(socket)),
        handler_(handler),
        lambda_(*this)
    {
    }

    // Start the asynchronous operation
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(stream_.get_executor(),
                      beast::bind_front_handler(
                          &session::do_read,
                          shared_from_this()));
    }

    void
    do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");

        // Send the response
        handle_request(handler_, std::move(req_), lambda_);
    }

    void
    on_write(
        bool close,
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        do_read();
    }

    void
    do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    //std::shared_ptr<std::string const> doc_root_;
    HandlerFunction handler_;

public:
	listener(net::io_context &ioc, tcp::endpoint endpoint,
			HandlerFunction handler) :
			ioc_(ioc), acceptor_(net::make_strand(ioc)), handler_(handler) {
		beast::error_code ec;

		// Open the acceptor
		acceptor_.open(endpoint.protocol(), ec);
		if (ec) {
			fail(ec, "open");
			return;
		}

		// Allow address reuse
		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec) {
			fail(ec, "set_option");
			return;
		}

		// Bind to the server address
		acceptor_.bind(endpoint, ec);
		if (ec) {
			fail(ec, "bind");
			return;
		}

		// Start listening for connections
		acceptor_.listen(net::socket_base::max_listen_connections, ec);
		if (ec) {
			fail(ec, "listen");
			return;
		}
	}

	// Start accepting incoming connections
	void run() {
		do_accept();
	}

private:
	void do_accept() {
		// The new connection gets its own strand
		acceptor_.async_accept(net::make_strand(ioc_),
				beast::bind_front_handler(&listener::on_accept,
						shared_from_this()));
	}

	void on_accept(beast::error_code ec, tcp::socket socket) {
		if (ec) {
			fail(ec, "accept");
		} else {
			// Create the session and run it
			std::make_shared<session>(std::move(socket), handler_)->run();
		}

		// Accept another connection
		do_accept();
	}
};

//------------------------------------------------------------------------------


using namespace SwitchTiming;

// Global io context for clean up
boost::asio::io_context ioc;
// Set if signal has been raised.
bool signal_seen {false};
// Put pointers to all equipment into a vector

Equipment equipment;


//------------------------------------------------------------------------------
class JSON_handler: public std::enable_shared_from_this<JSON_handler> {
public:
	JSON_handler(Equipment& equipment) :
			equipment_ { equipment } {
	}

	std::string PostHandler(beast::string_view path, beast::string_view json) {
		PLOG(plog::debug) << "path: " << path;
		PLOG(plog::debug) << "Json request: " << json;
		try {
			EquipmentCommands eq_cmd(path, json	);
			if ( eq_cmd.RunCommand(equipment))
				return std::string("Success");
			return std::string("Failure");
		} catch (std::exception &e){
			return std::string("JSON format error!");
		}
	}

	std::string GetHandler(beast::string_view path) {
		EquipmentStatus eq_status(equipment, path);
		result = eq_status.JSON_Results();
		return result;
	}
	std::string RequestHandler(http::verb method, beast::string_view path,
			beast::string_view body_str) {
		if (method == http::verb::get)
			return GetHandler(path);
		else
			return PostHandler(path, body_str);
	}
	std::shared_ptr<JSON_handler> getptr() {
		return shared_from_this();
	}
private:
	std::string result;
	Equipment& equipment_;
};



void terminate_handler(int signal_num){
	if ( ! signal_seen ){
		signal_seen = true;
		for ( auto eq: equipment){
			eq.second->SwitchOff();
		}
	}
	ioc.stop();
	exit(signal_num);
}
/// log usage
///   DBG_LOG(PLOG(plog::debug)<< c_id << " v:"<< protocol_version << "clean:"<< clean_session);
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
		std::cerr << "Usage:  <schedule-file-path> [fatal|warning|info|none|error|debug|verbose]\n";
		return 1;
	}

	enum plog::Severity severity { plog::none };
	if (argc == 3) {
		if (!strcmp(argv[2], "none"))
			severity = plog::none;
		else if (!strcmp(argv[2], "fatal"))
			severity = plog::fatal;
		else if (!strcmp(argv[2], "error"))
			severity = plog::error;
		else if (!strcmp(argv[2], "warning"))
			severity = plog::error;
		else if (!strcmp(argv[2], "info"))
			severity = plog::info;
		else if (!strcmp(argv[2], "debug"))
			severity = plog::debug;
		else if (!strcmp(argv[2], "verbose"))
			severity = plog::verbose;
	}
	std::cerr << "PLOG severity " << severity << std::endl;
	static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(severity, &consoleAppender);
	// Configure SI7021 temp and humidity sensor to monitor enclosure.
	GPIO::SI7021 si7021("Controller", I2C_INTERFACE, ioc, SI7021_READ_INTERVAL);
	// Configure Wireless Receiver/Transmitter and sensors.
	WirelessSerialSensors::WaterTemperatureRT water_sensor("WaterSensor", WATERTEMP_RT);
	WirelessSerialSensors::WaterTemperatureRT test_rt_sensor("TestSensor", TEST_RT);
	// Start Wireless handler and configure RT it is responsible for.
	WirelessSerialSensors::WirelessHandler rt_handler(ioc, WIRELESS_SERIAL_PORT, BAUD_RATE);
	rt_handler.ConfigureRT(WATERTEMP_RT, &water_sensor);
	rt_handler.ConfigureRT(TEST_RT, &test_rt_sensor);


	// Configure Relay switching, Push Buttons and Button pilot LEDs.
	MomentarySwitch mainPump("MainPump", MAIN_PUMP_GPIO, MAIN_PUMP_TOGGLE_GPIO, MAIN_PUMP_PILOT);
	PLOG(plog::debug)<< "MainPump setup";
	MomentarySwitch IC40("SWG", IC40_GPIO, IC40_TOGGLE_GPIO, IC40_PILOT_LED);
	PLOG(plog::debug)<< "IC40 Setup";
	MomentarySwitch boosterPump ("BoosterPump", BOOSTER_PUMP_GPIO, BOOSTER_PUMP_TOGGLE_GPIO, BOOSTER_PILOT);
	PLOG(plog::debug)<< "BoosterPump setup";
	MomentarySwitch poolLight ("Lighting", POOL_LIGHT_GPIO, POOL_LIGHT_TOGGLE_GPIO, POOL_LIGHT_PILOT );
	PLOG(plog::debug) << "MomentarySwitch created -- set conditions";
	std::vector<RelaySwitcher::RequiredState> mainPumpConditions {
	  {IC40, RelaySwitcher::SwitchRequest::OFF, RelaySwitcher::RelayState::OFF, RelaySwitcher::ForceState::FORCE_REQUIRED},
	  {boosterPump, RelaySwitcher::SwitchRequest::OFF, RelaySwitcher::RelayState::OFF, RelaySwitcher::ForceState::FORCE_REQUIRED}
	};
	mainPump.SetConditions(&mainPumpConditions);

	std::vector<RelaySwitcher::RequiredState> IC40Conditions {
	  {mainPump, RelaySwitcher::SwitchRequest::ON, RelaySwitcher::RelayState::ON, RelaySwitcher::ForceState::FORCE_REQUIRED}
	};
	IC40.SetConditions(&IC40Conditions);

	std::vector<RelaySwitcher::RequiredState> boosterConditions {
	  {mainPump, RelaySwitcher::SwitchRequest::ON, RelaySwitcher::RelayState::ON, RelaySwitcher::ForceState::FORCE_REQUIRED}
	};
	boosterPump.SetConditions(&boosterConditions);

	// Enable logic level shifter ( 3.3v to 5 v)
	GPIO::DigitalOut enable_logic_output(LOGIC_ENABLE);
	enable_logic_output.on();

	equipment["MainPump"] =  &mainPump;
	equipment["SWG"] =&IC40;
	equipment["BoosterPump"] = &boosterPump;
	equipment["Lighting"] = &poolLight;
	// Add sensors
	equipment["Controller"] = &si7021;
	equipment["WaterSensor"] = &water_sensor;
	equipment["TestSensor"] = &test_rt_sensor;
	// Initailize Http Server ---------------------------------------------------------------------
	// ToDo: Add these to configuration file.
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("8080"));

	std::shared_ptr json_handler = std::make_shared<JSON_handler>(equipment);
	auto request_handler = std::bind(&JSON_handler::RequestHandler, json_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	// Create and launch a listening port ----------------------------------------------------
	std::make_shared<listener>(ioc, tcp::endpoint{address, port},request_handler)
			->run();

	// Read Schedule file and start everything.
	try {
		PLOG(plog::debug) << "Schedule equipment";
		Schedule schedule(argv[1], equipment);

		schedule.RunSchedule(ioc);
		ioc.run();
		PLOG(plog::info)<< "PoolSmartz program exited";
		exit(0);
	} catch (const ScheduleFileException& e){
		PLOG(plog::error) << argv[1] << ": Unable to process schedule file.";
		exit (1);
	} catch (const std::exception& ex) {
	    PLOG(plog::error) << "Exception: " << ex.what () ;
	    exit (1);
	} catch (...) {
	    PLOG(plog::error) << "Unknown failure " ;
	    exit(1);
	}

}
