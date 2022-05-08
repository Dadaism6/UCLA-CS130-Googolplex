#include <fstream>
#include <iostream>
#include <boost/filesystem/path.hpp>

#include "request_handler_static.h"
#include "log.h"
#include "content_type.h"

namespace http = boost::beast::http;

status request_handler_static::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    INFO << ": Using static request handler\n";

    std::string prefix = get_prefix() + "/";
    size_t pos = request.target().find(prefix);
    // if found the given location, and the url starts with the given location,
    // and the string of location is smaller than the url, this is a valid path
    if (pos != std::string::npos && pos == 0 && request.target().length() > prefix.length()) {
        std::string path = get_dir() + "/" + std::string(request.target().substr(prefix.length()));
        // remove trailing slash
        while(path.length() >= 1 && path[path.length()-1] == '/') {
            path.pop_back();
        }
        INFO << get_client_ip() << ": Static request: trying to find: " << path << "\n";
        
        std::ifstream file(path, std::ios::binary);
        // read from file
        if (file.good()) {
            INFO << get_client_ip() << ": Reading data...\n";
            file.seekg(0, std::ios::end);
            std::string content;
            content.resize(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&content[0], content.size());

            // if found the file, set response message
            response.result(http::status::ok);
            set_content_type(path, response);
            response.body() = content;
            response.prepare_payload();
            INFO << get_client_ip() << ": Finish Setting Response\n";

            file.close();
            return true;
        }
        file.close();
        INFO << get_client_ip() << ": Static request: cannot find requested file: " << path << "\n";
    }    
    // cannot find the file
    response.result(http::status::not_found);
    response.set(http::field::content_type, "text/html");
    response.body() = not_found_msg;
    response.prepare_payload();

	return false;
}

std::string request_handler_static::set_content_type(std::string file_path, http::response<http::string_body>& response)
{
    std::string extension = "";
    boost::filesystem::path p(file_path);
    if (p.has_extension()) {
        extension = p.extension().string();
    }
    // if extension is empty string, will set content type to text/plain as default
    content_type c_type = content_type::get_content_type(extension);
    response.set(http::field::content_type, c_type.content_type_str);
    return extension;
}