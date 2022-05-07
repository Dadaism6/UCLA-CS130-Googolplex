#include "request_handler_factory.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "request_handler_not_found.h"

RequestHandlerFactory::RequestHandlerFactory(config_arg arg) : arg(arg) 
{
    INFO << "Invoke " << arg.handler_type << " and serve at location " << arg.location << "\n";
}

// Utilize polymorphism for factory method creation
request_handler* RequestHandlerFactory::create() 
{
    INFO << "Create " << arg.handler_type << " handler\n";
    if (arg.handler_type == "StaticHandler")
        return new request_handler_static(arg.location, arg.root);
    if (arg.handler_type == "EchoHandler")
        return new request_handler_echo(arg.location, arg.root);
    INFO << "No matching handler called " << arg.handler_type << " . Use NotFound handler instead\n";
    return new request_handler_not_found(arg.location, arg.root);
}
