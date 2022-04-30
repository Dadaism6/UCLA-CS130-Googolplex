#include <cstdlib>
#include <fstream>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

#include "request_handler_static.h"
#include "log.h"
#include "content_type.h"

http::server::reply request_handler_static::handle_request(Request request)
{
	char* in_data = request.in_data;
    std::string dir = request.dir;
    std::string inputsuffix = request.suffix;
    std::string client_ip = request.client_ip;
    INFO << "Using static request handler\n";
	http::server::reply rep;
	http::server::request req = get_request();

    std::string suffix = "/" + inputsuffix + "/";
    size_t pos = req.uri.find(suffix);
    if (pos != std::string::npos && pos == 0 && req.uri.length() > suffix.length()) {
        std::string path = dir + "/" + req.uri.substr(suffix.length());
        INFO << client_ip << ": Static request: trying to find: " << path << "\n";
        std::ifstream file(path, std::ios::binary);
        if (file.good()) {
            INFO << client_ip << ": Reading data...\n";
            file.seekg(0, std::ios::end);
            std::string content;
            content.resize(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&content[0], content.size());
            rep = http::server::reply::stock_reply(http::server::reply::ok);
            rep.content = content;
            rep.headers[content_length_field].value = std::to_string(content.length());

            set_content_type(path, rep);
            INFO << client_ip << ": Finish Setting Reply\n";
            file.close();
            return rep;

        }
        file.close();
    }    

    rep = http::server::reply::stock_reply(http::server::reply::not_found);
	return rep;
}

std::string request_handler_static::set_content_type(std::string file_path, http::server::reply& rep)
{
    std::string extension = "";
    boost::filesystem::path p(file_path);
    if (p.has_extension()) {
        extension = p.extension().string();
    }
    content_type c_type = content_type::get_content_type(extension);
    rep.headers[content_type_field].value = c_type.content_type_str;
    return extension;
}