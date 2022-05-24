#ifndef CONFIG_ARG_H
#define CONFIG_ARG_H

#include <string>
#include <set>

struct config_arg {
    std::string handler_type = "";
    std::string location = "";
    std::string root = "";
    std::string api_key = "";

    std::set<std::string> valid_handler_types = {
        "StaticHandler", "EchoHandler", "NotFoundHandler", "CrudHandler", "BlockHandler", "HealthHandler", "TextGenHandler"
    };
};
#endif // CONFIG_ARG_H