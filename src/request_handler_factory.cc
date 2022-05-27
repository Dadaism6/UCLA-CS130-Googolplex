#include "request_handler_factory.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "request_handler_not_found.h"
#include "request_handler_crud.h"
#include "request_handler_block.h"
#include "request_handler_health.h"
#include "request_handler_text_gen.h"
#include "log.h"

#include <boost/filesystem.hpp>

RequestHandlerFactory::RequestHandlerFactory(config_arg arg) : arg(arg) 
{
    INFO << "Invoke " << arg.handler_type << " and serve at location " << arg.location << "\n";
}

EchoHandlerFactory::EchoHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

StaticHandlerFactory::StaticHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

NotFoundHandlerFactory::NotFoundHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

CrudHandlerFactory::CrudHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {
    boost::filesystem::path rootpath(arg.root);
    //if root path (eg. /api) does not exist, create one
    if( !(boost::filesystem::exists(rootpath) && boost::filesystem::is_directory(rootpath)))
    {
        INFO << "CURD root path: " << arg.root << " does not exist! Creating\n";
        boost::filesystem::path rootFolder = arg.root;
        try {
            boost::filesystem::create_directory(rootFolder);
        } catch (const boost::filesystem::filesystem_error& e) {
            FATAL << "Bad root in config for CrudHandler: " << arg.root << "\n";
            exit(1);
        }
    }
}

BlockHandlerFactory::BlockHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

HealthHandlerFactory::HealthHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

TextGenHandlerFactory::TextGenHandlerFactory(config_arg arg) : RequestHandlerFactory(arg) {}

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

request_handler* CrudHandlerFactory::create()
{
    return new request_handler_crud(arg.location, arg.root, file_to_id);
}

request_handler* BlockHandlerFactory::create() 
{
    return new request_handler_block(arg.location, arg.root);
}

request_handler* HealthHandlerFactory::create() 
{
    return new request_handler_health(arg.location, arg.root);
}

request_handler* TextGenHandlerFactory::create() 
{
    boost::filesystem::path rootpath(arg.root);
    //if root path (eg. /api) does not exist, create one
    if( !(boost::filesystem::exists(rootpath) && boost::filesystem::is_directory(rootpath)))
    {
        INFO << "text_gen root path: " << arg.root << " does not exist! Creating\n";
        boost::filesystem::path rootFolder = arg.root;
        try {
            boost::filesystem::create_directory(rootFolder);
        } catch (const boost::filesystem::filesystem_error& e) {
            FATAL << "Bad root in config for TextGenHandler: " << arg.root << "\n";
            exit(1);
        }
    }

    return new request_handler_text_gen(arg.location, arg.root, arg.api_key);
}