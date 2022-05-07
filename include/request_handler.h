#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H


#include <cstdlib>
#include <map>
#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>

namespace http = boost::beast::http;

class request_handler 
{
    public:
        request_handler(std::string location, std::string root) {prefix_ = location; dir_ = root; }
        virtual bool handle_request(http::request<http::string_body> request, http::response<http::string_body>& response) = 0;

        std::string get_prefix() {return prefix_; }
        std::string get_dir() {return dir_; }
        std::string get_client_ip() {return client_ip_; }

        void set_client_ip(std::string client_ip) {client_ip_ = client_ip; }

        enum {content_length_field = 0, content_type_field = 1};

    private:
        std::string client_ip_;
        std::string prefix_;
        std::string dir_;
};

#endif  // REQUEST_HANDLER_H