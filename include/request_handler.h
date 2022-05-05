#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H


#include <cstdlib>
#include <map>
#include "http/request.h"
#include "http/reply.h"

struct Request {
    char* in_data = nullptr;
    std::string dir = "";
    std::string prefix = "";
    std::string client_ip = "";
};

class request_handler 
{

    public:
        request_handler(http::server::request& req, bool valid) {req_ = req; valid_ = valid;}
        virtual http::server::reply handle_request(Request request) = 0;
        http::server::request get_request() {return req_; }
        bool get_status() {return valid_; }
        enum {content_length_field = 0, content_type_field = 1};

    private:
        http::server::request req_;
        bool valid_;
};

#endif  // REQUEST_HANDLER_H