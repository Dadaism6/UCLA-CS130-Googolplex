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
        virtual request_handler* create() = 0;
        config_arg arg;
};

class EchoHandlerFactory : public RequestHandlerFactory
{
    public:
        EchoHandlerFactory(config_arg arg);
        request_handler* create();
};

class StaticHandlerFactory : public RequestHandlerFactory
{
    public:
        StaticHandlerFactory(config_arg arg);
        request_handler* create();
};

class NotFoundHandlerFactory : public RequestHandlerFactory
{
    public:
        NotFoundHandlerFactory(config_arg arg);
        request_handler* create();
};

#endif  // HANDLER_FACTORY_H