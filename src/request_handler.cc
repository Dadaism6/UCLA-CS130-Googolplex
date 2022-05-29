#include "request_handler.h"
#include "log.h"
#include <fstream>
#include <stdio.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace http = boost::beast::http;


bool request_handler::write_to_file(std::string path, std::string content) {
    if ( boost::filesystem::is_directory(path)) {
        return false;
    }
    std::ostringstream oss;     
    oss << content;
    std::string body = oss.str();
    oss.clear();
    std::ofstream file(path);
    file << body;
    file.close();
    return true;
}

bool request_handler::read_from_file(std::string path, std::string& content) {
    std::ifstream file(path, std::ios::binary);
    // read from file
    if (file.good()) {
        INFO << get_client_ip() << ": reading data at path: " << path << " ...\n";
        file.seekg(0, std::ios::end);
        content.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&content[0], content.size());
        file.close();
        return true;
    }
    return false;
}

bool request_handler::create_dir(std::string path) {
    if ( boost::filesystem::exists(path) && boost::filesystem::is_directory(path)) {
        return true;
    }
    try {
        boost::filesystem::create_directory(path);
    } 
    catch (const boost::filesystem::filesystem_error& e) {
        INFO << get_client_ip() << ": unable to create directory for " << path << "\n";
        return false;
    }
    INFO << get_client_ip() << ": created directory: " << path << "\n";
    return true;
}

// speicial parsing, if url == prefix/, return true with empty key, used for get list
bool request_handler::check_request_url(std::string url, std::string& key, bool for_get) {
    std::string prefix = get_prefix() + "/";
    INFO << get_client_ip() << ": prefix is: " << prefix << "  url is: " << url;
    size_t pos = url.find(prefix);
    if (pos != std::string::npos && pos == 0 && url.length() > prefix.length()) {
        key = std::string(url.substr(prefix.length()));
        //remove trailing slash
        while(key.length() >= 1 && key[key.length()-1] == '/') {
            key.pop_back();
        }
        return true;
    }
    else if(pos != std::string::npos && pos == 0 && url.length() == prefix.length()&& for_get) {
        key = "";
        return true;
    }
    else if(prefix.compare(url+"/") == 0){
        key = "";
        return true;
    }
    key = "";
    return false;
}