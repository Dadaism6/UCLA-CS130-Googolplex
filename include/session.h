#ifndef SESSION_H
#define SESSION_H

#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include "http/request.h"
#include "http/request_parser.h"
#include "http/reply.h"

using boost::asio::ip::tcp;

class session
{
	public:
		session(boost::asio::io_service& io_service);

		tcp::socket& socket();

		void start();

		void clear_data();
		void add_header(char* in_data, int bytes_transferred);
		void parse_request(char* request_data, int current_data_len);

	private:
		void handle_read(const boost::system::error_code& error,
			size_t bytes_transferred);

		void handle_write(const boost::system::error_code& error);
		
		tcp::socket socket_;
		int current_data_len_;
		// char last4bytes_[5];
		enum { max_length = 1024 };
		char in_data_[max_length];
		char response_content_[max_length * 2]; // make space for header
		char response_data_[max_length];
		char response_header_[max_length];

		http::server::request_parser request_parser_;
	
};

#endif  // SESSION_H