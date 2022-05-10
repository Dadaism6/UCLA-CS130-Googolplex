#include "session.h"
#include "log.h"

#include <boost/beast/core/buffers_to_string.hpp>


using boost::asio::ip::tcp;

namespace http = boost::beast::http;

session::session(boost::asio::io_service& io_service, std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes) : socket_(io_service), routes(routes)
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
	if (data_len < 0 || data_len > max_length || request_data == nullptr) 
		return false;
	boost::beast::error_code ec;
	http::request_parser<http::string_body> parser;
	// set eager as true so that request body is also parsed
	parser.eager(true);
	// parse request
	size_t n = parser.put(boost::asio::buffer(std::string(request_data)), ec);
	if (ec) return false;
	request = parser.release();
	return true;
}

/* search the location-root binding recursively in addrmap constructed by config parser 
   return if found or not */
bool session::search_addr_binding(std::string url, std::string& location)
{
	char delimiter = '/'; 
	// find last occurance of "/" to get longest match 
	size_t pos = url.find_last_of(delimiter); 
	if (pos != 0 && pos != std::string::npos) {
		std::string sub_url = url.substr(0, pos);
		if (routes.find(sub_url) != routes.end()) {
			location = sub_url;
			return true;
		}
		else { // current prefix not found, recursively find shorter prefixes
			return search_addr_binding(url.substr(0, pos), location);
		}
	} else if (pos == 0 && routes.find("/") != routes.end()) { // matched with beginning "/"
		location = "/";
		return true;
	}
	return false; // npos
}


http::response<http::string_body> session::generate_response(char* request_data, int data_len)
{
	std::string location;
	http::request<http::string_body> request;
	http::response<http::string_body> response;
	
	if (parse_request(request_data, data_len, request)) {
		std::string target_url = std::string(request.target());
		target_url.append("/");
		if(search_addr_binding(target_url, location))
		{
			// Use factory method for dispatching
			request_handler_ = routes[location]->create();
			request_handler_ -> set_client_ip(client_ip_);

			if(request_handler_ -> handle_request(request, response))
				INFO << "Successfully handled the request, return response to client " << client_ip_ << "\n";
			else
				INFO << "Something went wrong when handling the request, return response to client " << client_ip_ << "\n";
			delete request_handler_;  // short life cycle request handler
			return response;
		}
	}

	response.result(http::status::bad_request);
	return response;
}

// get the reply from http request
std::string session::get_reply(char* request_data, int data_len)
{
	http::response<http::string_body> response = generate_response(request_data, data_len);
	std::ostringstream oss;
    oss << response;
    std::string reply = oss.str();
    oss.clear();
	return reply;
}