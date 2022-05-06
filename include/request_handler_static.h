#ifndef REQUEST_HANDLER_STATIC_H
#define REQUEST_HANDLER_STATIC_H


#include <cstdlib>
#include <map>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

class request_handler_static : public request_handler 
{

    public:
        using request_handler::request_handler;
        void handle_request(Request request, http::server::reply& reply);
    
    private:
        std::string set_content_type(std::string file_path, http::server::reply& rep);
};

#endif  // REQUEST_HANDLER_STATIC_H