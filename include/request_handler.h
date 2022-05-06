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
        request_handler(http::request<http::string_body>& req, bool valid) {req_ = req; valid_ = valid; }
        virtual void handle_request(http::request<http::string_body> request, http::response<http::string_body>& response) = 0;
        http::request<http::string_body> get_request() {return req_; }
        bool get_status() {return valid_; }
        std::string get_client_ip() {return client_ip_; }
        void set_client_ip(std::string client_ip) {client_ip_ = client_ip; }
        std::string get_prefix() {return prefix_; }
        void set_prefix(std::string prefix) {prefix_ = prefix; }
        std::string get_dir() {return dir_; }
        void set_dir(std::string dir) {dir_ = dir; }
        enum {content_length_field = 0, content_type_field = 1};

    private:
        http::request<http::string_body> req_;

        bool valid_;

        std::string client_ip_;
        std::string prefix_;
        std::string dir_;
};

#endif  // REQUEST_HANDLER_H