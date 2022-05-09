#include "request_handler_factory.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "request_handler_not_found.h"
#include "log.h"

RequestHandlerFactory::RequestHandlerFactory(config_arg arg) : arg(arg) 
{
    INFO << "Invoke " << arg.handler_type << " and serve at location " << arg.location << "\n";
}

EchoHandlerFactory::EchoHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

StaticHandlerFactory::StaticHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

NotFoundHandlerFactory::NotFoundHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

// Utilize polymorphism for factory method creation
request_handler* EchoHandlerFactory::create() 
{
    return new request_handler_echo(arg.location, arg.root);
}

request_handler* StaticHandlerFactory::create() 
{
    return new request_handler_static(arg.location, arg.root);
}

request_handler* NotFoundHandlerFactory::create() 
{
    return new request_handler_not_found(arg.location, arg.root);
}
