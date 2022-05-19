#include "request_handler_health.h"
#include "log.h"

namespace http = boost::beast::http;

status request_handler_health::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{	
	std::string target = std::string(request.target());
	INFO << get_client_ip() << ": Using heath request handler\n";

	response.result(http::status::ok);
	response.body() = "OK"; 
	response.set(http::field::content_type, "text/plain");
	response.prepare_payload();

	return true;
}
