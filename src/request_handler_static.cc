#include <cstdlib>
#include <fstream>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"

#include "request_handler_static.h"
#include "log.h"
http::server::reply request_handler_static::handle_request(char* in_data, std::string dir, std::string inputsuffix)
{
    INFO << "Using static request handler\n";
	http::server::reply rep;
	
    std::string suffix = "/" + inputsuffix + "/";
    size_t pos = req_.uri.find(suffix);
    if (pos != std::string::npos && pos == 0 && req_.uri.length() > suffix.length()) {
        std::string path = dir + "/" + req_.uri.substr(suffix.length());
        INFO << "Static request: trying to find: " << path << "\n";
        std::ifstream file(path, std::ios::binary);
        if (file.good()) {
            INFO << "Reading data...\n";
            file.seekg(0, std::ios::end);
            std::string content;
            content.resize(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&content[0], content.size());
            rep = http::server::reply::stock_reply(http::server::reply::ok);
            rep.content = content;
            rep.headers[content_length_field].value = std::to_string(content.length());

            set_content_type(path, rep);
            INFO << "Finish Setting Reply\n";
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
    std::string content_type = "";
    boost::filesystem::path p(file_path);
    if (p.has_extension()) {
        extension = p.extension().string();
        if (extension == ".html" || extension == ".htm") 
            content_type = "text/html";
        else if (extension == ".txt")
            content_type = "text/plain";
        else if (extension == ".jpg" || extension == ".jpeg")
            content_type = "image/jpeg";
        else if (extension == ".png")
            content_type = "image/png";
        else if (extension == ".gif")
            content_type = "image/gif";
        else if (extension == ".zip")
            content_type = "application/zip";
    } else {
        content_type = "text/plain";
    }
    rep.headers[content_type_field].value = content_type;
    return extension;
}