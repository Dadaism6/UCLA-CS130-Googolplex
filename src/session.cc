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
#include "request_handler_not_found.h"
#include "log.h"
#include "config_arg.h"

using boost::asio::ip::tcp;


session::session(boost::asio::io_service& io_service, std::map<std::string, config_arg> addrmap) : socket_(io_service), addrmap(addrmap)
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

/* search the location-root binding recursively in addrmap constructed by config parser, 
			return if found or not */
bool session::search_addr_binding(std::string url, config_arg& args)
{
	char delimiter = '/'; 
	// find last occurance of "/" to get longest match 
	size_t pos = url.find_last_of(delimiter); 
	if (pos != 0 && pos != std::string::npos) {
		std::string sub_url = url.substr(0, pos);
		if (addrmap.find(sub_url) != addrmap.end()) {
			args = addrmap.at(sub_url);
			return true;
		}
		else { // current prefix not found, recursively find shorter prefixes
			return search_addr_binding(url.substr(0, pos), args);
		}
	} else if (pos == 0) { // matched with beginning "/"
		if (addrmap.find("/") != addrmap.end()) {
			args = addrmap.at("/");
			return true;
		}
		return false;
	} else { // npos
		return false;
	}
}

// get the reply from http request
http::server::reply session::get_reply(char* request_data, int data_len)
{

	http::server::request request;
	bool valid = parse_request(request_data, data_len, request);

	config_arg args;

	if (valid) {
		// append "/" at the end to handle cases like /echo 
		std::string target_url = request.uri;
		target_url.append("/");
		if (search_addr_binding(target_url, args)) {
			INFO << "Invoke " << args.handler_type << " and serve at location " << args.location << "\n";
			if (args.handler_type == "StaticHandler")
				request_handler_ = new request_handler_static(request, valid);
			else if (args.handler_type == "EchoHandler")
				request_handler_ = new request_handler_echo(request, valid);
			else if (args.handler_type == "404Handler")
				request_handler_ = new request_handler_not_found(request, valid);	
			else
				valid = false;
		} else {
			WARNING << "Cannot find suitable prefixes for the given location: " << request.uri << " from the config file.\n";
			valid = false;
		}

	} else {
		WARNING << "Invalid HTTP request\n";
		valid = false;
	}

	if ( !valid)
		request_handler_ = new request_handler_echo(request, valid);
	
	Request raw_request;
    raw_request.in_data = request_data;
    raw_request.dir = args.root;
    raw_request.prefix = args.location;
    raw_request.client_ip = client_ip_;

	http::server::reply rep = request_handler_ -> handle_request(raw_request);
	
	delete request_handler_;
	request_handler_ = NULL;
	return rep;
}
