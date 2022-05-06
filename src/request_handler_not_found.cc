#include <cstdlib>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

#include "request_handler_not_found.h"
#include "log.h"

void request_handler_not_found::handle_request(Request request, http::server::reply& reply)
{
	std::string client_ip = request.client_ip;
	INFO << client_ip << ": Using not_found request handler\n";
	if (get_status())
		reply = http::server::reply::stock_reply(http::server::reply::not_found);
	else
		reply = http::server::reply::stock_reply(http::server::reply::bad_request);
	return;
}
