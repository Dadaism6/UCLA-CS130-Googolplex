#ifndef REQUEST_HANDLER_STATIC_H
#define REQUEST_HANDLER_STATIC_H

#include "request_handler.h"

namespace http = boost::beast::http;

class request_handler_static : public request_handler 
{
    public:
        using request_handler::request_handler;
        void handle_request(http::request<http::string_body> request, http::response<http::string_body>& response);

    private:
        std::string set_content_type(std::string file_path, http::response<http::string_body>& response);
};

#endif  // REQUEST_HANDLER_STATIC_H