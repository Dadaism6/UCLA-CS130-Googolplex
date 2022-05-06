#include "request_handler_not_found.h"
#include "log.h"

namespace http = boost::beast::http;

void request_handler_not_found::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
	INFO << get_client_ip() << ": Using not_found request handler\n";
	response.set(http::field::content_type, "text/html");
	if (get_status()) {
		response.result(http::status::not_found);
		response.body() = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
	}
	else {
		response.result(http::status::bad_request);
		response.body() = "<html><head><title>Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>";
	}

	response.prepare_payload();
	return;
}
