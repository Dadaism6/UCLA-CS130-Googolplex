#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <cstdlib>
#include <map>
#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>

namespace http = boost::beast::http;

typedef bool status;

class request_handler 
{
    public:
        request_handler(std::string location, std::string root) {prefix_ = location; dir_ = root; }
        virtual status handle_request(http::request<http::string_body> request, http::response<http::string_body>& response) = 0;

        std::string get_prefix() {return prefix_; }
        std::string get_dir() {return dir_; }
        std::string get_client_ip() {return client_ip_; }

        void set_client_ip(std::string client_ip) {client_ip_ = client_ip; }

        // common helper function
        bool write_to_file(std::string path, std::string content);
        bool read_from_file(std::string path, std::string& content); 
        bool create_dir(std::string path);
        bool check_request_url(std::string url, std::string& key, bool for_get);
        enum {content_length_field = 0, content_type_field = 1};

    protected:
        std::string bad_request_msg = "<html><head><title>Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>";
        std::string not_found_msg = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
        std::string internal_server_error_msg = "<html><head><title>Internal Server Error</title></head><body><h1>500 Internal Server Error</h1></body></html>";

    private:
        std::string client_ip_;
        std::string prefix_;
        std::string dir_;
};

#endif  // REQUEST_HANDLER_H