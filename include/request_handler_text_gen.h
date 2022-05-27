#ifndef REQUEST_HANDLER_TEXT_GEN_H
#define REQUEST_HANDLER_TEXT_GEN_H

#include "request_handler.h"
#include <string>

namespace http = boost::beast::http;

class request_handler_text_gen: public request_handler 
{

    public:
        request_handler_text_gen(std::string location, std::string root, std::string api_key) 
            : request_handler(location, root), api_key(api_key) {}
        status handle_request(http::request<http::string_body> request, http::response<http::string_body>& response);
    
    private:
        int text_prompt_max_len = 50;
        int title_max_len = 20;
        std::string api_key;
        bool curl_api(std::string input, std::string& output);
        bool parse_raw_output(const std::string& raw_output, std::string& output);
        bool parse_post_body(std::string body, std::map<std::string, std::string>& token_map);

        bool check_prompts_validity(std::string text);
        bool check_title_validity(std::string title);
        std::string dot2underscore(std::string text);

        void prepare_bad_request_response(http::response<http::string_body>& response);
        void prepare_not_found_response(http::response<http::string_body>& response);
        void prepare_internal_server_error_response(http::response<http::string_body>& response);

        bool handle_post_request(http::request<http::string_body> request, http::response<http::string_body>& response);
        bool handle_options_request(http::request<http::string_body> request, http::response<http::string_body>& response);
};

#endif  // REQUEST_HANDLER_TEXT_GEN_H