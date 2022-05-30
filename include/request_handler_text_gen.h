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

        // error messages
        std::string post_format_msg = "<h4>Bad Request</h4>Required POST request body format: title=xxx&prompt=xxx";
        std::string valid_input_msg = "<h4>Bad Request</h4><h5>Invalid title:</h5> <ul><li>at least 1 character and at most 20 characters\
            </li> <li>only contains letters, numeric characters, special characters (\".\", \"_\", \"-\", and \" \")</li></ul><h5>Invalid text prompt:\
            </h5> <ul><li>at least 1 character and at most 50 characters</li><li>only contains letters, numeric characters, special characters (\"?\", \",\", \"_\", \"-\", and \" \")</li></ul>";
        std::string api_communication_msg = "<h4>Bad Request</h4>Whoops, errors happen in the process: <ul>\
            <li>communication with the text generation API</li></ul>Please wait for a moment and try again";
        std::string file_system_msg = "<h4>Bad Request</h4>Whoops, errors happen in the process: <ul>\
            <li>storing the data to server\'s filesystem</li></ul>Please wait for a moment and try again";
        std::string invalid_url_msg = "<h4>Bad Request</h4>The URL you entered is not valid";
        
        // data processing
        bool curl_api(std::string input, std::string& output);
        bool parse_raw_output(const std::string& raw_output, std::string& output);
        bool parse_post_body(std::string body, std::map<std::string, std::string>& token_map);
        std::string dot2underscore(std::string text);

        // checking the validity of inputs
        bool check_prompts_validity(std::string text);
        bool check_title_validity(std::string title);

        // encapsulations of common responses
        void prepare_bad_request_response(http::response<http::string_body>& response);
        void prepare_not_found_response(http::response<http::string_body>& response);
        void prepare_internal_server_error_response(http::response<http::string_body>& response);

        // access filesystem
        bool get_ip_file_list(std::string path, std::string suffix, http::response<http::string_body>& response);
        bool get_title_file(std::string path, std::string suffix, http::response<http::string_body>& response, bool remove);

        // handlers
        bool handle_post_request(http::request<http::string_body> request, http::response<http::string_body>& response);
        bool handle_get_request(http::request<http::string_body> request, http::response<http::string_body>& response);
        bool handle_delete_request(http::request<http::string_body> request, http::response<http::string_body>& response);
        bool handle_options_request(http::request<http::string_body> request, http::response<http::string_body>& response);

};

#endif  // REQUEST_HANDLER_TEXT_GEN_H