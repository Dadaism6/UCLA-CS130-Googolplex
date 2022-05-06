#ifndef REQUEST_HANDLER_NOT_FOUND_H
#define REQUEST_HANDLER_NOT_FOUND_H

#include "request_handler.h"

namespace http = boost::beast::http;

class request_handler_not_found: public request_handler 
{
    public:
        using request_handler::request_handler;
        void handle_request(http::request<http::string_body> request, http::response<http::string_body>& response);
};

#endif  // REQUEST_HANDLER_NOT_FOUND_H