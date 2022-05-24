#ifndef HANDLER_FACTORY_H
#define HANDLER_FACTORY_H

#include "request_handler.h"
#include "config_arg.h"
#include <vector>


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

class CrudHandlerFactory : public RequestHandlerFactory
{
    public:
        CrudHandlerFactory(config_arg arg);
        request_handler* create();
    private:
        std::map<std::string, std::vector<int>> file_to_id;
};

class BlockHandlerFactory : public RequestHandlerFactory
{
    public:
        BlockHandlerFactory(config_arg arg);
        request_handler* create();
};

class HealthHandlerFactory : public RequestHandlerFactory
{
    public:
        HealthHandlerFactory(config_arg arg);
        request_handler* create();
};

class TextGenHandlerFactory : public RequestHandlerFactory
{
    public:
        TextGenHandlerFactory(config_arg arg);
        request_handler* create();
};

#endif  // HANDLER_FACTORY_H