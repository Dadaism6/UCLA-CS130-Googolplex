#ifndef REQUEST_HANDLER_ECHO_H
#define REQUEST_HANDLER_ECHO_H


#include <cstdlib>
#include <map>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

class request_handler_echo: public request_handler 
{

    public:
        using request_handler::request_handler;
        http::server::reply handle_request(char* in_data, std::string dir, std::string suffix, std::string client_ip);

};

#endif  // REQUEST_HANDLER_ECHO_H