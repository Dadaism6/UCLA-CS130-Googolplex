#ifndef REQUEST_HANDLER_HEALTH_H
#define REQUEST_HANDLER_HEALTH_H

#include "request_handler.h"

namespace http = boost::beast::http;

class request_handler_health: public request_handler 
{

    public:
        using request_handler::request_handler;
        status handle_request(http::request<http::string_body> request, http::response<http::string_body>& response);

};

#endif  // REQUEST_HANDLER_HEALTH_H