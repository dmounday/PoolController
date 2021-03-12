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

#ifndef HTTPSERVERLIB_H_
#define HTTPSERVERLIB_H_

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/utility/string_view.hpp>
//#include <string_view>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/asio.hpp>



namespace beast = boost::beast;
// from <boost/beast.hpp>
namespace http = beast::http;
// from <boost/beast/http.hpp>
namespace net = boost::asio;
// from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;
// from <boost/asio/ip/tcp.hpp>

using HandlerFunction = std::function<std::string(http::verb,
                                                  beast::string_view,
                                                  beast::string_view)>;

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session> {
  // This is the C++11 equivalent of a generic lambda.
  // The function object is used to send an HTTP message.
  struct send_lambda {
    session &self_;

    explicit send_lambda (session &self) :
        self_ (self) {
    }

    template<bool isRequest, class Body, class Fields>
      void operator() (http::message<isRequest, Body, Fields> &&msg) const {
        // The lifetime of the message has to extend
        // for the duration of the async operation so
        // we use a shared_ptr to manage it.
        auto sp = std::make_shared<http::message<isRequest, Body, Fields>> (
            std::move (msg));
        // Store a type-erased version of the shared
        // pointer in the class to keep it alive.
        self_.res_ = sp;
        // Write the response
        http::async_write (
            self_.stream_,
            *sp,
            beast::bind_front_handler (&session::on_write,
                                       self_.shared_from_this (),
                                       sp->need_eof ()));
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
  session (tcp::socket &&socket, HandlerFunction handler);
  // Start the asynchronous operation
  void run ();
  void do_read ();
  void on_read (beast::error_code ec, std::size_t bytes_transferred);
  void on_write (bool close, beast::error_code ec, std::size_t bytes_transferred);
  void do_close ();
};

class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  //std::shared_ptr<std::string const> doc_root_;
  HandlerFunction handler_;

public:
  listener (net::io_context &ioc, tcp::endpoint endpoint,
            HandlerFunction handler);
  // Start accepting incoming connections
  void run();

private:
  void do_accept ();
  void on_accept (beast::error_code ec, tcp::socket socket);
};

#endif /* HTTPSERVERLIB_H_ */
