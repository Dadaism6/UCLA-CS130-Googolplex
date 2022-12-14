#ifndef SESSION_H
#define SESSION_H

#include "request_handler.h"
#include "request_handler_factory.h"

#include <cstdlib>
#include <map>
#include <memory>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

using boost::asio::ip::tcp;
namespace http = boost::beast::http;

class session
{
	public:
		session(boost::asio::io_service& io_service, std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes);

		tcp::socket& socket();

		virtual void start(); // read from socket and write to buffer and call handle_read
		virtual void recycle(); // when error, delete the object
		virtual void read(); // read from buffer and write to socket and call handle_write

		/* reply to http request by reading the reply and writing to socket, 
			then listen to incoming requests, return status */
		virtual bool handle_read(const boost::system::error_code& error,
			size_t bytes_transferred); 

		/* read http request from the socket and write to buffer, return status */
		virtual bool handle_write(const boost::system::error_code& error);

		// get the reply from http request
		std::string get_reply(char* request_data, int data_len);
		http::response<http::string_body> generate_response(char* request_data, int data_len);

	private:
		std::string client_ip_;
		tcp::socket socket_;
		std::string rep_;
		enum { max_length = 1024,  content_length_field = 0, content_type_field = 1};
		std::string response_metric = "[ResponseMetrics]";
		char in_data_[max_length];

		request_handler* request_handler_;
		std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes;

		/* parse the request and construct a request, return whether request is valid */
		bool parse_request(char* request_data, int data_len, http::request<http::string_body>& request);

		/* search the location-root binding recursively in addrmap constructed by config parser, 
			return if found or not */
		bool search_addr_binding(std::string url, std::string& location);
};

#endif  // SESSION_H