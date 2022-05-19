#include "request_handler_block.h"
#include "log.h"

namespace http = boost::beast::http;

status request_handler_block::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{	
	// set the blocking time according to in_data body
	std::string target = std::string(request.target());
	// sleep 1s by default
	int block_time = 1;
	// the sleep time is specified in url (/sleep/2 means sleep for 2s)
	try {
		block_time = std::stoi(target.substr(target.find_last_of('/') + 1));
	}
	// sleep time not specified / wrong format, use default
	catch (const std::exception&) {
	}
	INFO << get_client_ip() << ": Using block request handler to block " << std::to_string(block_time) <<"s\n";
	sleep(block_time);

	response.result(http::status::ok);
	response.body() = "Sleep" + std::to_string(block_time) + "s!\n"; 
	response.set(http::field::content_type, "text/plain");
	response.prepare_payload();

	
	return true;
}
