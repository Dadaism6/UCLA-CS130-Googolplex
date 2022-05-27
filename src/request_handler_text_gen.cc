#include "request_handler_text_gen.h"
#include "log.h"

#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace http = boost::beast::http;

/* references: 
    https://curl.se/libcurl/c/
    https://gist.github.com/alghanmi/c5d7b761b2c9ab199157
    https://stackoverflow.com/questions/51923585/how-to-convert-curl-command-with-f-option-to-libcurl
    https://github.com/nlohmann/json
    
*/

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string request_handler_text_gen::dot2underscore(std::string text)
{
    std::string text_copy = text;
    std::replace(text_copy.begin(), text_copy.end(), '.', '_');
    return text_copy;
}

// these functions can be lifted to the parent class
void request_handler_text_gen::prepare_bad_request_response(http::response<http::string_body>& response)
{
    response.result(http::status::bad_request);
    response.set(http::field::content_type, "text/html");
    response.body() = bad_request_msg;
    response.prepare_payload();
}

void request_handler_text_gen::prepare_not_found_response(http::response<http::string_body>& response)
{
    response.result(http::status::not_found);
    response.set(http::field::content_type, "text/html");
    response.body() = not_found_msg;
    response.prepare_payload();
}

void request_handler_text_gen::prepare_internal_server_error_response(http::response<http::string_body>& response)
{
    response.result(http::status::internal_server_error);
    response.body() = internal_server_error_msg;
    response.set(http::field::content_type, "text/html");
    response.prepare_payload();
}

bool request_handler_text_gen::curl_api(std::string input, std::string& output)
{
    const char* input_c_str = input.c_str();
    CURL *curl = curl_easy_init();
    struct curl_slist *list = NULL;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepai.org/api/text-generator");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

        // -F 'text=TEXT'
        curl_mime *mime = curl_mime_init(curl);
        curl_mimepart *part = curl_mime_addpart(mime);
        curl_mime_data(part, input_c_str, CURL_ZERO_TERMINATED);
        curl_mime_name(part, "text");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        // -H 'api-key:xxx'
        std::string cur_api_key = "api-key:" + api_key;
        list = curl_slist_append(list, cur_api_key.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        curl_mime_free(mime);
        curl_slist_free_all(list);

        curl = NULL;
        mime = NULL;
        list = NULL;
        return true;
    }
    curl = NULL;
    return false;
}

bool request_handler_text_gen::parse_raw_output(const std::string& raw_output, std::string& output)
{   
    try {
        json j_string = raw_output;
        auto cpp_string = j_string.get<std::string>();
        auto j = json::parse(cpp_string);
        output = j["output"];
        return true;
    } 
    // catch all the exceptions
    catch (...) {
        WARNING << "cannot parse JSON output or bad response from the third-party API, response from that API: " << raw_output << "\n";
        return false;
    }
}

bool request_handler_text_gen::check_prompts_validity(std::string text)
{
    if (text.length() <= 0 || text.length() > text_prompt_max_len)
        return false;
    std::__cxx11::regex str_expr ("^[a-zA-Z0-9? ,_-]+$");
    if (regex_match(text, str_expr))
        return true;
    return false;
}

bool request_handler_text_gen::check_title_validity(std::string title)
{
    if (title.length() <= 0 || title.length() > title_max_len)
        return false;
    std::__cxx11::regex str_expr ("^[a-zA-Z0-9. _-]+$");
    if (regex_match(title, str_expr))
        return true;
    return false;
}

bool request_handler_text_gen::handle_post_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    //TODO: parse input fields from front-end (title, text prompt)
    std::string input = request.body();

    std::string title = "dummy_title";
    std::string text_prompt = input;

    std::string raw_output;
    std::string output;

    if ( (!check_prompts_validity(text_prompt)) || (!check_title_validity(title)))
    {
        prepare_bad_request_response(response); // error message to user?
        return false;
    }

    if (curl_api(text_prompt, raw_output))
    {
        if (parse_raw_output(raw_output, output)) {
            response.result(http::status::ok);
            response.body() = output; 
            response.set(http::field::content_type, "text/plain");
            response.prepare_payload();

            std::string folder = get_dir() + "/" + dot2underscore(get_client_ip());
            if (create_dir(folder))
            {
                std::string path = folder + "/" + title;
                std::string entry = "title: " + title + "\n\ntext prompt: " + text_prompt + "\n\ntext generation:\n" + output;
                if (write_to_file(path, entry))
                    return true;
                else
                    WARNING << "cannot store user's input from " << get_client_ip() << "with title " << title << "\n";
            } else {
                WARNING << "cannot create dir for " << get_client_ip() << "\n";
            }
        }
    }

    prepare_internal_server_error_response(response);
	return false;
}

status request_handler_text_gen::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{	
	std::string target = std::string(request.target());
	INFO << get_client_ip() << ": Using text generation request handler\n";

    auto method = request.method();

    switch (method) {
        // post, get, put, delete
        case http::verb::post: {
            return handle_post_request(request, response);
        }
        case http::verb::get: {
            //TODO
            break;
        }
        case http::verb::put: {
            //TODO
            break;
        }
        case http::verb::delete_: {
            //TODO
            break;
        }
        default:
        {
            prepare_bad_request_response(response);
            return false;
        }
    }

    prepare_not_found_response(response);
    return false;
}