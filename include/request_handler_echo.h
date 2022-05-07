#ifndef REQUEST_HANDLER_ECHO_H
#define REQUEST_HANDLER_ECHO_H

#include "request_handler.h"

namespace http = boost::beast::http;

class request_handler_echo: public request_handler 
{

    public:
        using request_handler::request_handler;
        bool handle_request(http::request<http::string_body> request, http::response<http::string_body>& response);

};

#endif  // REQUEST_HANDLER_ECHO_H