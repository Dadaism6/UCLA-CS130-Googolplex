#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "session.h"
#include "http/request_parser.h"
#include "http/reply.h"
#include "request_handler.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "log.h"
using boost::asio::ip::tcp;


session::session(boost::asio::io_service& io_service, std::map<std::string, std::string> addrmap) : socket_(io_service), addrmap(addrmap)
{
	memset(in_data_, 0, max_length);
}

tcp::socket& session::socket()
{
	return socket_;
}

// read from socket and write to buffer and call handle_read
void session::start()
{
	client_ip_ = socket_.remote_endpoint().address().to_string();
	INFO << client_ip_ << ": Connection started" << "\n";
	socket_.async_read_some(boost::asio::buffer(in_data_, max_length), boost::bind(&session::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

// read from buffer and write to socket and call handle_write
void session::read()
{
	boost::asio::async_write(socket_, rep_.to_buffers(), boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
}

// when error, delete the object
void session::recycle()
{
	delete this;
}

// reply to http request by reading the reply and writing to socket, then listen to incoming requests, return status
bool session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		rep_ = get_reply(in_data_, bytes_transferred);
		memset(in_data_, 0, max_length); // clear to prepare for new incoming data
		read();
	}
	else
	{
		recycle();
	}
	return (!error);
}

// read http request from the socket and write to buffer, return status
bool session::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		start();
	}
	else
	{
		recycle();
	}
	return (!error);
}

// parse the request and construct a request, return whether request is valid
bool session::parse_request(char* request_data, int data_len, http::server::request& request)
{
	bool valid;
	char* dummy;
	http::server::request_parser::result_type result;
	if (data_len < 0 || data_len > max_length || request_data == nullptr) 
	{
		valid = false;
	} else {
		http::server::request_parser request_parser;
		std::tie(result, dummy) = request_parser.parse(request, request_data, request_data + data_len);
		if ((request.uri == "/") || (result == http::server::request_parser::good && request.uri[0] == '/'))
			valid = true;
		else
			valid = false;
	}
	return valid;
}

// search the location-root binding in addrmap constructed by config parser, return if found or not
bool session::search_addr_binding(std::string location, std::string& root)
{
	if (addrmap.find(location) != addrmap.end()) {
		root = addrmap.at(location);
		return true;
	} else {
		return false;
	}
}

// get the prefix, or mode of the server (e.g. static, static1, echo) in url of http request
std::string session::get_prefix(std::string url)
{
	std::string delimiter = "/";
	if (url.length() < 1) return "";
	std::string actual_url = url.substr(1);
	size_t pos = actual_url.find(delimiter);
	if (pos == std::string::npos) 
		pos = actual_url.length();
	std::string mode = actual_url.substr(0, pos);
	return mode;
}

// get the reply from http request
http::server::reply session::get_reply(char* request_data, int data_len)
{
	std::string dir = "";
	std::string location = "";

	http::server::request request;
	bool valid = parse_request(request_data, data_len, request);
	bool static_server = false;

	std::string echo_path = "/echo";
	std::string static_path = "";

	if (valid) {
		std::string prefix = get_prefix(request.uri);
		std::string slash_prefix = "/" + prefix;
		// the prefix of url matches the location of echo path in config
		if (search_addr_binding("", echo_path) && (slash_prefix == echo_path)) {
			INFO << "Bind echo server to: " << echo_path << "\n";
		} 
		// no location of echo path in config, the prefix of url matches default location
		else if (!(search_addr_binding("", echo_path)) && (slash_prefix == echo_path || slash_prefix == "/")) {
			WARNING << "Cannot find specified echo path in the config file, use default /echo or /\n";
		} 
		// the prefix of url matches the location of static path in config
		else if (search_addr_binding(slash_prefix, static_path)) {
			dir = static_path;
			location = prefix;
			static_server = true;
		} 
		// cannot find the given prefix in config/default settings, not valid request
		else {
			WARNING << "Cannot find the given location: " << prefix << " from the config file.\n";
			valid = false;
		}
	}

	if (static_server)
		request_handler_ = new request_handler_static(request, valid);
	else
		request_handler_ = new request_handler_echo(request, valid);
	
	Request raw_request;
    raw_request.in_data = request_data;
    raw_request.dir = dir;
    raw_request.suffix = location;
    raw_request.client_ip = client_ip_;
	http::server::reply rep = request_handler_ -> handle_request(raw_request);
	
	delete request_handler_;
	request_handler_ = NULL;
	return rep;
}
