#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "session.h"
#include "http/request_parser.h"
#include "http/reply.h"

using boost::asio::ip::tcp;


session::session(boost::asio::io_service& io_service) : socket_(io_service)
{
	memset(in_data_, 0, max_length);
}

tcp::socket& session::socket()
{
	return socket_;
}

void session::start()
{
	socket_.async_read_some(boost::asio::buffer(in_data_, max_length),
		boost::bind(&session::handle_read, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}


void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		http::server::reply rep = add_header(in_data_, bytes_transferred);
		memset(in_data_, 0, max_length); // clear to prepare for new incoming data
		boost::asio::async_write(socket_,
				rep.to_buffers(),
				boost::bind(&session::handle_write, this,
					boost::asio::placeholders::error)
		);
	}
	else
	{
		delete this;
	}
}

void session::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		socket_.async_read_some(boost::asio::buffer(in_data_, max_length),
			boost::bind(&session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}
	else
	{
		delete this;
	}
}


// return a reply struct by adding header to the content
http::server::reply session::add_header(char* in_data, int bytes_transferred)
{
	bool valid = parse_request(in_data, bytes_transferred);
	http::server::reply rep;
	if (valid) rep = http::server::reply::stock_reply(http::server::reply::ok);
	else rep = http::server::reply::stock_reply(http::server::reply::bad_request);

	std::string response_content(in_data);

	rep.content = response_content; // set the content to be the http request
	rep.headers[0].value = std::to_string(bytes_transferred);	// content length
	rep.headers[1].value = "text/plain";	// content type

	return rep;
}

// check whether the http request is valid or not 
bool session::parse_request(char* request_data, int current_data_len)
{
	char* dummy; // want to ignore the result, use a dummy char*
	http::server::request_parser::result_type result;
	http::server::request request;
	std::tie(result, dummy) = request_parser_.parse(request, request_data, request_data + current_data_len);
	
	
	if (result == http::server::request_parser::good) return true;
	else return false;
}