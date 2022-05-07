#ifndef HANDLER_FACTORY_H
#define HANDLER_FACTORY_H

#include "request_handler.h"
#include "config_arg.h"
#include "log.h"

enum HandlerType 
{
    Echo_Handler,
    Static_Handler,
    Not_Found_Handler
};

class RequestHandlerFactory 
{
    public:
        RequestHandlerFactory(config_arg arg);
        request_handler* create();
    private:
        config_arg arg;
};

#endif  // HANDLER_FACTORY_H