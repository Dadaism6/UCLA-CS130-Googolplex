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

		virtual void start();
		virtual void recycle();
		virtual void read();
		http::server::reply add_header(char* in_data, int bytes_transferred);
		bool parse_request(char* request_data, int current_data_len);

		bool handle_read(const boost::system::error_code& error,
			size_t bytes_transferred);
		bool handle_write(const boost::system::error_code& error);

	private:
		tcp::socket socket_;
		http::server::reply rep_;
		enum { max_length = 1024,  content_length_field = 0, content_type_field = 1};
		char in_data_[max_length];

		http::server::request_parser request_parser_;
	
};

#endif  // SESSION_H