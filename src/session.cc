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
	current_data_len_ = 0;
	// memset(last4bytes_, 0, 5);
	memset(in_data_, 0, max_length);
	memset(response_data_, 0, max_length);
	memset(response_content_, 0, max_length);
	memset(response_header_, 0, max_length);
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
		add_header(in_data_, bytes_transferred);
		clear_data();
		boost::asio::async_write(socket_,
				boost::asio::buffer(response_content_, strlen(response_content_)),
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

void session::clear_data()
{
	current_data_len_ = 0;
	memset(in_data_, 0, max_length); // reset in buffer
	memset(response_header_, 0, max_length); // reset response header
	memset(response_data_, 0, max_length); // reset response data
}

void session::add_header(char* in_data, int bytes_transferred)
{
	strncpy(response_data_ + current_data_len_, in_data, bytes_transferred);

	current_data_len_ += bytes_transferred;

	parse_request(response_data_, current_data_len_);

	memset(response_content_, 0, max_length * 2); // reset out buffer
	strncpy(response_content_, response_header_, strlen(response_header_));
	strncpy(response_content_ + strlen(response_header_), response_data_, current_data_len_);
}

void session::parse_request(char* request_data, int current_data_len)
{
	char* dummy;
	http::server::request_parser::result_type result;
	http::server::request request;
	// std::cout << "request data: " << request_data << " request length: " << current_data_len << std::endl;
	std::tie(result, dummy) = request_parser_.parse(request, request_data, request_data + current_data_len);

	if (result == http::server::request_parser::good)
	{
		strcpy(response_header_, ("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(current_data_len) + "\r\n\r\n").c_str());
	} else if (result == http::server::request_parser::bad) {
		strcpy(response_header_, ("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(current_data_len) + "\r\n\r\n").c_str());
	} 
}