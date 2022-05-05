#include <cstdlib>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

#include "request_handler_not_found.h"
#include "log.h"

http::server::reply request_handler_not_found::handle_request(Request request)
{
	std::string client_ip = request.client_ip;
	INFO << client_ip << ": Using not_found request handler\n";
	return http::server::reply::stock_reply(http::server::reply::not_found);
}
