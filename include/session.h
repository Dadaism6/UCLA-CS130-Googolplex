#ifndef SESSION_H
#define SESSION_H

#include <cstdlib>
#include <map>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include "http/request.h"
#include "http/request_parser.h"
#include "http/reply.h"
#include "request_handler.h"

using boost::asio::ip::tcp;

class session
{
	public:
		session(boost::asio::io_service& io_service, std::map<std::string, std::string> addrmap);

		tcp::socket& socket();

		virtual void start();
		virtual void recycle();
		virtual void read();
		
		http::server::reply parse_request(char* request_data, int current_data_len);

		virtual bool handle_read(const boost::system::error_code& error,
			size_t bytes_transferred);
		virtual bool handle_write(const boost::system::error_code& error);

	private:
		std::string client_ip_;
		tcp::socket socket_;
		http::server::reply rep_;
		enum { max_length = 1024,  content_length_field = 0, content_type_field = 1};
		char in_data_[max_length];

		request_handler* request_handler_;
		std::map<std::string, std::string>  addrmap;
};

#endif  // SESSION_H