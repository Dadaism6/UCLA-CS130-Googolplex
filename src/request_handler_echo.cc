#include "request_handler_echo.h"
#include "log.h"

namespace http = boost::beast::http;

void request_handler_echo::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
	INFO << get_client_ip() << ": Using echo request handler\n";

	// first construct a response based on the validity of http response
	if (get_status()) {
		response.result(http::status::ok);
	}
	else {
		response.result(http::status::bad_request);
		response.set(http::field::content_type, "text/html");
		response.body() = "<html><head><title>Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>";
		WARNING << get_client_ip() << ": Bad echo request\n"; 
	}

	// set the content of the response according to in_data char array
	std::ostringstream oss;
	oss << request;
	std::string content = oss.str();
	oss.clear();

	response.body() = content; 
	response.set(http::field::content_type, "text/plain");
	response.prepare_payload();
	
	return;
}
