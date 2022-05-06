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

void request_handler_static::handle_request(Request request, http::server::reply& reply)
{
    INFO << "Using static request handler\n";
	http::server::request req = get_request();

    std::string prefix = request.prefix + "/";
    size_t pos = req.uri.find(prefix);
    /* if found the given location, and the url starts with the given location,
     and the string of location is smaller than the url, this is a good path */
    if (pos != std::string::npos && pos == 0 && req.uri.length() > prefix.length()) {
        std::string path = request.dir + "/" + req.uri.substr(prefix.length());
        INFO << request.client_ip << ": Static request: trying to find: " << path << "\n";
        std::ifstream file(path, std::ios::binary);
        // read from file
        if (file.good()) {
            INFO << request.client_ip << ": Reading data...\n";
            file.seekg(0, std::ios::end);
            std::string content;
            content.resize(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&content[0], content.size());

            // if found the file, set reply
            reply = http::server::reply::stock_reply(http::server::reply::ok);
            reply.content = content;
            reply.headers[content_length_field].value = std::to_string(content.length());

            set_content_type(path, reply);
            INFO << request.client_ip << ": Finish Setting Reply\n";
            file.close();
            return;

        }
        file.close();
    }    
    // cannot find the file
    reply = http::server::reply::stock_reply(http::server::reply::not_found);
	return;
}

std::string request_handler_static::set_content_type(std::string file_path, http::server::reply& rep)
{
    std::string extension = "";
    boost::filesystem::path p(file_path);
    if (p.has_extension()) {
        extension = p.extension().string();
    }
    // if extension is empty string, will set content type to text/plain as default
    content_type c_type = content_type::get_content_type(extension);
    rep.headers[content_type_field].value = c_type.content_type_str;
    return extension;
}