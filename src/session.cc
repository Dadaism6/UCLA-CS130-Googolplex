#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

#include "session.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "request_handler_not_found.h"
#include "log.h"
#include "config_arg.h"


using boost::asio::ip::tcp;

namespace http = boost::beast::http;

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
	boost::asio::async_write(socket_, boost::asio::buffer(rep_), boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
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
bool session::parse_request(char* request_data, int data_len, http::request<http::string_body>& request)
{
	// bool valid;
	// char* dummy;
	// http::server::request_parser::result_type result;
	// if (data_len < 0 || data_len > max_length || request_data == nullptr) 
	// {
	// 	valid = false;
	// } else {
	// 	simple::request_parser request_parser;
	// 	std::tie(result, dummy) = request_parser.parse(request, request_data, request_data + data_len);
	// 	if ((request.uri == "/") || (result == simple::request_parser::good && request.uri[0] == '/'))
	// 		valid = true;
	// 	else
	// 		valid = false;
	// }
	// return valid;
	boost::beast::error_code ec;
	http::request_parser<http::string_body> p;
	p.put(boost::asio::buffer(std::string(request_data)), ec);
	if (ec) return false;
	request = p.release();
	return true;
   
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

request_handler* create_handler(http::request<http::string_body>& req, bool valid, config_arg arg) {
    INFO << "Invoke " << arg.handler_type << " and serve at location " << arg.location << "\n";
    if (arg.handler_type == "StaticHandler")
        return new request_handler_static(req, valid);
    if (arg.handler_type == "EchoHandler")
        return new request_handler_echo(req, valid);
    return new request_handler_not_found(req, valid);
}

// get the reply from http request
std::string session::get_reply(char* request_data, int data_len)
{
	config_arg args;
	http::request<http::string_body> request;
	http::response<http::string_body> response;

	bool valid = parse_request(request_data, data_len, request);
	
	if (valid) {
		std::string target_url = std::string(request.target());
		target_url.append("/");
		valid = search_addr_binding(target_url, args);
	}

	request_handler_ = create_handler(request, valid, args);
	request_handler_ -> set_client_ip(client_ip_);
	request_handler_ -> set_prefix(args.location);	
	request_handler_ -> set_dir(args.root); 

	request_handler_ -> handle_request(request, response);

	delete request_handler_;
	request_handler_ = NULL;

	std::ostringstream oss;
    oss << response;
    std::string reply = oss.str();
    oss.clear();
	return reply;
}