#Ambient Weather Station Client

Poll the live data on the Ambient ObserverIP and format data into an Influxdb line protocol message and send it via UDP to a Telegraf listener.

Uses Boost ASIO. HTTP client is modified Boost ASIO example.
C++17
 
