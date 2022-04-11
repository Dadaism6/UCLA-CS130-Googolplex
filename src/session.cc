#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "session.h"
#include "http/request_parser.h"
#include "http/reply.h"

using boost::asio::ip::tcp;


session::session(boost::asio::io_service& io_service) : socket_(io_service)
{
	current_data_len = 0;
	memset(last4bytes_, 0, 5);
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
		strncpy(response_data_ + current_data_len, in_data_, bytes_transferred);
		current_data_len += bytes_transferred;
		if (strlen(response_data_) >= 4)
			strncpy(last4bytes_, response_data_ + current_data_len - 4, 4);
		else 
			memset(last4bytes_, 0, 5);
		if (!strcmp(last4bytes_, "\r\n\r\n")) { // done reading
			strcpy(response_header_, ("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length:" + std::to_string(current_data_len) + "\r\n\r\n").c_str());
			strncpy(response_content_, response_header_, strlen(response_header_));
			strncpy(response_content_ + strlen(response_header_), response_data_, current_data_len);
			boost::asio::async_write(socket_,
				boost::asio::buffer(response_content_, strlen(response_content_)),
				boost::bind(&session::handle_write, this,
					boost::asio::placeholders::error)
			);
		}
		else {
			memset(in_data_, 0, max_length);
			socket_.async_read_some(boost::asio::buffer(in_data_, max_length),
				boost::bind(&session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)
			);
		}
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
		memset(in_data_, 0, max_length); // reset in buffer
		memset(response_content_, 0, max_length * 2); // reset out buffer
		memset(response_header_, 0, max_length); // reset response header
		memset(response_data_, 0, max_length); // reset response data
		memset(last4bytes_, 0, 5); // reset \r\n\r\n checker
		current_data_len = 0;
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

