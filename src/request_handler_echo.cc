#include <cstdlib>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

#include "request_handler_echo.h"
#include "log.h"
http::server::reply request_handler_echo::handle_request(Request request)
{
	char* in_data = request.in_data;
	INFO << request.client_ip << ": Using echo request handler\n";
	http::server::reply rep;

	// first construct a reply based on the validity of http response
	if (get_status()) 
	{
        rep = http::server::reply::stock_reply(http::server::reply::ok);
	}
	else 
	{
        rep = http::server::reply::stock_reply(http::server::reply::bad_request);
		WARNING << request.client_ip << ": Bad echo request\n"; 
	}

	// set the content of the response according to in_data char array
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
