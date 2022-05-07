#include "request_handler_not_found.h"
#include "log.h"

namespace http = boost::beast::http;

bool request_handler_not_found::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
	INFO << get_client_ip() << ": Using not_found request handler\n";
	response.set(http::field::content_type, "text/html");
	response.result(http::status::not_found);
	response.body() = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
	response.prepare_payload();
	return true;
}
