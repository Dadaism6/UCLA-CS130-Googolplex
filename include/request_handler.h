#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H


#include <cstdlib>
#include <map>
#include "http/request.h"
#include "http/reply.h"

class request_handler 
{

    public:
        request_handler(http::server::request& req, bool valid) {req_ = req; valid_ = valid;}
        virtual http::server::reply handle_request(char* in_data, std::string dir, std::string suffix, std::string client_ip) = 0;

        http::server::request req_;
        bool valid_;
        enum {content_length_field = 0, content_type_field = 1};
};

#endif  // REQUEST_HANDLER_H