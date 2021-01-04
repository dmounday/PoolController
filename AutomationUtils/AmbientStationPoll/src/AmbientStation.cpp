//
// AmbientStation client
//
//

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

const std::string GET_VERB { "Get" };
const std::string LIVE_DATA { "/livedata.htm" };
/// HTTP Client to poll Ambient Wether Observer for current conditions.
/// This HTTP client polls the Observer and scraps the values for each reading.
/// It formats the labels and values into a Influxdb line protocol message and
/// send its via UDP to a Telegrapf listener.
///
class Client {
public:
  /// Constructor to create a Client object.
  /// \param io_context asio io context reference.
  /// \param server Reference to Observer server address or name.
  /// \param tg_server Telegraf IP address or name
  /// \param tg_port Telegraf listener UDP port.
  ///
  Client (boost::asio::io_context &io_context, const std::string &server,
          const std::string &tg_server, const std::string tg_port) :
      resolver_ { io_context }, socket_ { io_context }, to_resolver_ {
          io_context }, to_s_ (io_context, boost::asio::ip::udp::v4())

  {
    endpoints_ = to_resolver_.resolve (tg_server,
                                       tg_port);
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    std::ostream request_stream (&request_);
    request_stream << GET_VERB << " " << LIVE_DATA << " HTTP/1.1\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    resolver_.async_resolve (
        server,
        "http",
        [&] (const boost::system::error_code &err,
             const tcp::resolver::results_type endpoints) {
          if (!err) {
            DoConnect (endpoints);
          }
        });

  }

private:
  void DoConnect (const tcp::resolver::results_type &endpoints) {
    // Attempt a connection to each endpoint in the list until we
    // successfully establish a connection.
    boost::asio::async_connect (
        socket_,
        endpoints,
        [&] (const boost::system::error_code &err,
             const boost::asio::ip::tcp::endpoint &endpoint) {
          if (!err)
            DoRequest ();
          else
            std::cerr << "DoConnect: Error: " << err.message () << std::endl;
        });
    //boost::bind (&Client::handle_connect, this,
    //             boost::asio::placeholders::error)
    //);
  }

  void DoRequest () {
    // The connection was successful. Send the request.
    boost::asio::async_write (
        socket_,
        request_,
        [&] (const boost::system::error_code &err, std::size_t bytes_xfered) {
          if (!err) {
            // Read the response status line. The response_ streambuf will
            // automatically grow to accommodate the entire line. The growth may be
            // limited by passing a maximum size to the streambuf constructor.
            boost::asio::async_read_until (
                socket_,
                response_,
                "\r\n",
                [&] (const boost::system::error_code &err,
                     std::size_t bytes_xfered) {
                  if (!err && bytes_xfered > 0)
                    DoReadStatusLine (bytes_xfered);
                  else
                    std::cerr << "DoRequest async_read_until: Error: "
                        << err.message () << std::endl;

                });
          } else {
            std::cerr << "DoRequest async_write: Error: " << err.message ()
                << "\n";
          }
        });
  }

  void DoReadStatusLine (std::size_t bytes_xfered) {
    // Check that response is OK.
    std::istream response_stream (&response_);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline (response_stream, status_message);
    if (!response_stream || http_version.substr (0, 5) != "HTTP/") {
      std::cerr << "Invalid response\n";
      return;
    }
    if (status_code != 200) {
      std::cerr << "Response returned with status code ";
      std::cerr << status_code << "\n";
      return;
    }
    // Read the response headers, which are terminated by a blank line.
    boost::asio::async_read_until (
        socket_,
        response_,
        "\r\n\r\n",
        [&] (const boost::system::error_code &err, std::size_t bytes_xfered) {
          if (!err) {
            // Process the response headers.
            std::istream response_stream (&response_);
            std::string header;
            //while (std::getline (response_stream, header) && header != "\r")
            //  std::cout << header << "\n";
            //std::cout << "\n";

            // Write whatever content we already have to output.
            //if (response_.size () > 0)
            //  std::cout << &response_;
            DoReadContent ();
          } else {
            std::cerr << "DoReadStatusLine Error: " << err.message ()
                << std::endl;
          }
        });

  }

  void DoReadContent () {
    std::cerr << "DoReadContent\n";
    boost::asio::async_read (
        socket_, response_, boost::asio::transfer_all (),
        [&] (const boost::system::error_code &err, std::size_t bytes_xfered) {
          if (!err) {
            // Write all of the data that has been read so far.
            std::cerr << &response_;
            DoReadContent ();
          } else if (err != boost::asio::error::eof) {
            std::cerr << "Error: " << err << "\n";
          } else {
            std::cout << "size: " << response_.size () << std::endl;
            boost::asio::streambuf::const_buffers_type bufs = response_.data ();
            auto bb = &(*boost::asio::buffers_begin (bufs));
            std::string_view parse_buffer (bb, response_.size ());
            std::string::size_type s = parse_buffer.find (FORM_LIVE_DATA);
            if (s != std::string::npos) {
              FindValues (parse_buffer, s);
              SendData ();
            } else
              std::cerr << "data tag not found" << std::endl;
          }
        });
  }

  void FindValues (const std::string_view b, std::string::size_type p) {
    std::string::size_type endform = b.find ("/form", p);
    while (p < endform) {
      std::string::size_type s = b.find (INPUT_NAME, p);
      s += INPUT_NAME.size ();
      std::string::size_type e = b.find ("\"", s); // s..e is quoted name
      if (e != std::string::npos) {
        std::string mstr { b.substr (s, e - s) };
        auto measure = std::find_if (
            std::begin (fields_), std::end (fields_),
            [&mstr] (std::pair<std::string, std::string> &f) {
              return f.first == mstr;
            });
        if (measure != std::end (fields_)) {
          std::string::size_type v = b.find (VALUE, e);
          if (v != std::string::npos) {
            v += VALUE.size ();
            std::string::size_type ve = b.find ("\"", v);
            if (ve != std::string::npos) {
              measure->second = b.substr (v, ve - v);
            }
            p = ve;
          }
        } else
          p = e;
      } else
        break;
    }

  }

  void SendData () {
    std::string influx { "weather,station=ambient " };
    bool first { true };
    for (auto &p : fields_) {
      if (first) {
        first = false;
      } else {
        influx += ',';
      }
      influx += p.first;
      influx += "=";
      influx += p.second;
    }
    influx += ' ';
    auto now = std::chrono::high_resolution_clock::now ();
    influx += std::to_string (
        std::chrono::duration_cast<std::chrono::nanoseconds> (
            now.time_since_epoch ()).count ());
    influx += '\n';
    std::cout << influx << std::endl;
    try {
      std::size_t sz = to_s_.send_to (
          boost::asio::buffer (influx, influx.size ()), *endpoints_.begin ());
      if (sz == 0)
        std::cerr << "send_to: failed to write data" << std::endl;
    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what () << std::endl;
    }

  }
  const std::string FORM_LIVE_DATA { "form name=\"livedata\"" };
  const std::string INPUT_NAME { "input name=\"" };
  const std::string VALUE { "value=\"" };

  std::vector<std::pair<std::string, std::string>> fields_ { { "inTemp", " " },
      { "inHumi", " " }, { "AbsPress", " " }, { "RelPress", " " }, { "outTemp",
          " " }, { "outHumi", " " }, { "windir", " " }, { "avgwind", " " }, {
          "gustspeed", " " }, { "dailygust", " " }, { "solarrad", " " }, {
          "uvi", " " }, { "rainofhourly", " " }, { "eventrain", " " } };

  tcp::resolver resolver_;
  tcp::socket socket_;
  udp::resolver to_resolver_;
  udp::socket to_s_;
  udp::resolver::results_type endpoints_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
};

boost::asio::io_context io_context;
std::string station_ip;
std::string telegraf_ip;
std::string telegraf_port;
int sample_interval;
std::unique_ptr<Client> client;
/**
 *
 * @param e  error_code
 * @param *t pointer to running timer.
 */
void PollStation (const boost::system::error_code &e,
                   boost::asio::steady_timer *t) {

  client = std::make_unique<Client> (io_context, station_ip, telegraf_ip,
                                     telegraf_port);
  t->expires_at (t->expiry () + boost::asio::chrono::seconds (sample_interval));
  t->async_wait (bind (PollStation, boost::asio::placeholders::error, t));
}

int main (int argc, char *argv[]) {
  try {
    if (argc != 5) {
      std::cerr << "Usage: " << argv[0]
          << " <station-ip> <telegraf-ip> <telegraf-port> <sample-time-sec>\n";
      std::cerr << "Example:\n";
      std::cerr << "   " << argv[0] << " 192.168.3.72 localhost 8094 60"
          << std::endl;
      return 1;
    }
    station_ip = argv[1];
    telegraf_ip = argv[2];
    telegraf_port = argv[3];
    sample_interval = atoi (argv[4]);

    boost::asio::steady_timer t (
        io_context, boost::asio::chrono::seconds (sample_interval));
    t.async_wait (bind (PollStation, boost::asio::placeholders::error, &t));

    io_context.run ();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what () << "\n";
  }

  return 0;
}

