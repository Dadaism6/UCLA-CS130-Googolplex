#include <cstdlib>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

#include "request_handler_echo.h"

http::server::reply request_handler_echo::handle_request(char* in_data, std::string dir)
{
	http::server::reply rep;
	if (valid_) 
        rep = http::server::reply::stock_reply(http::server::reply::ok);
	else 
        rep = http::server::reply::stock_reply(http::server::reply::bad_request);
	if (in_data == nullptr) {
		rep.content = "";
		rep.headers[content_length_field].value = "0";
	} else {
		std::string response_content(in_data);
		rep.content = response_content; // set the content to be the http request
		rep.headers[content_length_field].value = std::to_string(strlen(in_data));
	}
	rep.headers[content_type_field].value = "text/plain";	// content type
	return rep;
}
