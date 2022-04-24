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

void session::start()
{
	client_ip_ = socket_.remote_endpoint().address().to_string();
	INFO << client_ip_ << ": Connection started" << "\n";
	socket_.async_read_some(boost::asio::buffer(in_data_, max_length), boost::bind(&session::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void session::read()
{
	boost::asio::async_write(socket_, rep_.to_buffers(), boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
}

void session::recycle()
{
	delete this;
}

bool session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		rep_ = parse_request(in_data_, bytes_transferred);
		memset(in_data_, 0, max_length); // clear to prepare for new incoming data
		read();
	}
	else
	{
		recycle();
	}
	return (!error);
}

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

// check whether the http request is valid or not 
http::server::reply session::parse_request(char* request_data, int current_data_len)
{
	bool valid = false;
	char* dummy; // want to ignore the result, use a dummy char*
	http::server::request_parser::result_type result;
	http::server::request request;
	
	bool static_server = false;
	std::string dummy_dir = ""; // should be configurable
	std::string suffix = "";
	if (current_data_len < 0 || current_data_len > max_length || request_data == nullptr) 
	{
		valid = false;
	} else {
		std::tie(result, dummy) = request_parser_.parse(request, request_data, request_data + current_data_len);

		if (request.uri == "/")
			valid = true;	
		else if (result == http::server::request_parser::good && request.uri[0] == '/') {
			valid = true;
			std::string delimiter = "/";
			std::string actual_uri = request.uri.substr(1);
			size_t pos = actual_uri.find(delimiter);
			if (pos == std::string::npos) 
				pos = actual_uri.length();
			std::string mode = actual_uri.substr(0, pos);
			if(mode == "echo"){
				valid = true;
			}
			else{
				std::string slash_mode = "/" + mode;
				if (addrmap.find(slash_mode) != addrmap.end()) {
					dummy_dir = addrmap.at(slash_mode);
					suffix = mode;
					request_handler_ = new request_handler_static(request, valid);
					static_server = true;
    			}
				else{
					WARNING << "Cannot find the given address: " << mode << " from the config file.\n";
					valid = false;
				}
			}
		}
	}
	if ( !static_server)
		request_handler_ = new request_handler_echo(request, valid);
	http::server::reply rep = request_handler_ -> handle_request(request_data, dummy_dir, suffix, client_ip_);
	delete request_handler_;
	request_handler_ = NULL;
	return rep;
}