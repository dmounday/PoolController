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

#include "HTTPServerLib.h"
#include "plog/Log.h"
#include "plog/Appenders/ConsoleAppender.h"


// Return a reasonable mime type based on the extension of a file.
beast::string_view
mime_type (beast::string_view path)
{
  using beast::iequals;
  auto const ext = [&path]
  {
    auto const pos = path.rfind(".");
    if(pos == beast::string_view::npos)
    return beast::string_view {};
    return path.substr(pos);
  }();
  if (iequals (ext, ".htm"))
    return "text/html";
  if (iequals (ext, ".html"))
    return "text/html";
  if (iequals (ext, ".php"))
    return "text/html";
  if (iequals (ext, ".css"))
    return "text/css";
  if (iequals (ext, ".txt"))
    return "text/plain";
  if (iequals (ext, ".js"))
    return "application/javascript";
  if (iequals (ext, ".json"))
    return "application/json";
  if (iequals (ext, ".xml"))
    return "application/xml";
  if (iequals (ext, ".swf"))
    return "application/x-shockwave-flash";
  if (iequals (ext, ".flv"))
    return "video/x-flv";
  if (iequals (ext, ".png"))
    return "image/png";
  if (iequals (ext, ".jpe"))
    return "image/jpeg";
  if (iequals (ext, ".jpeg"))
    return "image/jpeg";
  if (iequals (ext, ".jpg"))
    return "image/jpeg";
  if (iequals (ext, ".gif"))
    return "image/gif";
  if (iequals (ext, ".bmp"))
    return "image/bmp";
  if (iequals (ext, ".ico"))
    return "image/vnd.microsoft.icon";
  if (iequals (ext, ".tiff"))
    return "image/tiff";
  if (iequals (ext, ".tif"))
    return "image/tiff";
  if (iequals (ext, ".svg"))
    return "image/svg+xml";
  if (iequals (ext, ".svgz"))
    return "image/svg+xml";
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
template<class Body, class Allocator, class Send>
  void handle_request (HandlerFunction handler,
                       http::request<Body, http::basic_fields<Allocator>> &&req,
                       Send &&send) {
    PLOG(plog::debug);
    // Returns a bad request response
    auto const bad_request = [&req] (beast::string_view why) {
      http::response<http::string_body> res { http::status::bad_request,
          req.version () };
      res.set (http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set (http::field::content_type, "text/html");
      res.keep_alive (req.keep_alive ());
      res.body () = std::string (why);
      res.prepare_payload ();
      return res;
    };

    // Make sure we can handle the method
    if (req.method () != http::verb::get && req.method () != http::verb::post)
      return send (bad_request ("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target ().empty () || req.target ()[0] != '/'
        || req.target ().find ("..") != beast::string_view::npos)
      return send (bad_request ("Illegal request-target"));
    beast::string_view req_body = req.body ();
    std::string response_body = handler (req.method (), req.target (),
                                         req_body);
    PLOG(plog::debug) << "Response: " << response_body;
    auto const size = response_body.length ();
    // Respond to GET request
    http::response<http::string_body> res { std::piecewise_construct,
        std::make_tuple (std::move (response_body)), std::make_tuple (
            http::status::ok, req.version ()) };
    res.set (http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set (http::field::content_type, mime_type (".json"));
    res.content_length (size);
    res.keep_alive (req.keep_alive ());
    return send (std::move (res));

  }

//------------------------------------------------------------------------------

// Report a failure
void fail (beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message () << "\n";
}

// Handles an HTTP server connection

session::session (tcp::socket &&socket, HandlerFunction handler) :
    stream_ (std::move (socket)), handler_ (handler), lambda_ (*this) {
}

// Start the asynchronous operation
void session::run () {
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.
  net::dispatch (
      stream_.get_executor (),
      beast::bind_front_handler (&session::do_read, shared_from_this ()));
}

void session::do_read () {
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  req_ = { };

  // Set the timeout.
  stream_.expires_after (std::chrono::seconds (30));

  // Read a request
  http::async_read (
      stream_, buffer_, req_,
      beast::bind_front_handler (&session::on_read, shared_from_this ()));
}

void session::on_read (beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused (bytes_transferred);

  // This means they closed the connection
  if (ec == http::error::end_of_stream)
    return do_close ();

  if (ec)
    return fail (ec, "read");

  // Send the response
  handle_request (handler_, std::move (req_), lambda_);
}

void session::on_write (bool close, beast::error_code ec,
                        std::size_t bytes_transferred) {
  boost::ignore_unused (bytes_transferred);

  if (ec)
    return fail (ec, "write");

  if (close) {
    // This means we should close the connection, usually because
    // the response indicated the "Connection: close" semantic.
    return do_close ();
  }

  // We're done with the response so delete it
  res_ = nullptr;

  // Read another request
  do_read ();
}

void session::do_close () {
  // Send a TCP shutdown
  beast::error_code ec;
  stream_.socket ().shutdown (tcp::socket::shutdown_send, ec);

  // At this point the connection is closed gracefully
}

//------------------------------------------------------------------------------

listener::listener (net::io_context &ioc, tcp::endpoint endpoint,
                    HandlerFunction handler) :
    ioc_ (ioc), acceptor_ (net::make_strand (ioc)), handler_ (handler)
{
  beast::error_code ec;
  // Open the acceptor
  acceptor_.open (endpoint.protocol (), ec);
  if (ec) {
    fail (ec, "open");
    return;
  }

  // Allow address reuse
  acceptor_.set_option (net::socket_base::reuse_address (true), ec);
  if (ec) {
    fail (ec, "set_option");
    return;
  }

  // Bind to the server address
  acceptor_.bind (endpoint, ec);
  if (ec) {
    fail (ec, "bind");
    return;
  }

  // Start listening for connections
  acceptor_.listen (net::socket_base::max_listen_connections, ec);
  if (ec) {
    fail (ec, "listen");
    return;
  }
}

// Start accepting incoming connections
void listener::run () {
  do_accept ();
}

void listener::do_accept () {
  // The new connection gets its own strand
  acceptor_.async_accept (
      net::make_strand (ioc_),
      beast::bind_front_handler (&listener::on_accept, shared_from_this ()));
}

void listener::on_accept (beast::error_code ec, tcp::socket socket) {
  if (ec) {
    fail (ec, "accept");
  } else {
    // Create the session and run it
    std::make_shared<session> (std::move (socket), handler_)->run ();
  }

  // Accept another connection
  do_accept ();
}


