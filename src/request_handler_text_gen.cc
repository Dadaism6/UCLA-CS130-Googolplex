#include "request_handler_text_gen.h"
#include "log.h"

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <map>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using json = nlohmann::json;

namespace http = boost::beast::http;

/* references: 
    https://curl.se/libcurl/c/
    https://gist.github.com/alghanmi/c5d7b761b2c9ab199157
    https://stackoverflow.com/questions/51923585/how-to-convert-curl-command-with-f-option-to-libcurl
    https://github.com/nlohmann/json
    https://www.boost.org/doc/libs/1_36_0/doc/html/string_algo/usage.html#id3483755
    
*/


// helper function used to get the data from curl
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// replace dot with underscore (used to transform client ip like 127.0.0.1 to 127_0_0_1, more suitable name as a folder)
std::string request_handler_text_gen::dot2underscore(std::string text)
{
    std::string text_copy = text;
    std::replace(text_copy.begin(), text_copy.end(), '.', '_');
    return text_copy;
}

// encapsulation of a bad request response
void request_handler_text_gen::prepare_bad_request_response(http::response<http::string_body>& response)
{
    response.result(http::status::bad_request);
    response.set(http::field::content_type, "text/html");
    response.body() = bad_request_msg;
    response.prepare_payload();
}

// encapsulation of a not found response
void request_handler_text_gen::prepare_not_found_response(http::response<http::string_body>& response)
{
    response.result(http::status::not_found);
    response.set(http::field::content_type, "text/html");
    response.body() = not_found_msg;
    response.prepare_payload();
}

// encapsulation of an internal server error response
void request_handler_text_gen::prepare_internal_server_error_response(http::response<http::string_body>& response)
{
    response.result(http::status::internal_server_error);
    response.body() = internal_server_error_msg;
    response.set(http::field::content_type, "text/html");
    response.prepare_payload();
}

// send request to third-party api using curl
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

// parse the raw output from the third-party api
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
        WARNING << get_client_ip() << ": cannot parse JSON output or bad response from the third-party API, response from that API: " << raw_output << "\n";
        return false;
    }
}

// valid prompts: 0 < size <= 50; only has alphabets, numbers, "?", " ", ",", "_", "-"
bool request_handler_text_gen::check_prompts_validity(std::string text)
{
    if (text.length() <= 0 || text.length() > text_prompt_max_len)
        return false;
    std::__cxx11::regex str_expr ("^[a-zA-Z0-9? ,_-]+$");
    if (regex_match(text, str_expr))
        return true;
    return false;
}

// valid title: 0 < size <= 20; only has alphabets, numbers, ".", " ", "_", "-"
bool request_handler_text_gen::check_title_validity(std::string title)
{
    if (title.length() <= 0 || title.length() > title_max_len)
        return false;
    std::__cxx11::regex str_expr ("^[a-zA-Z0-9. _-]+$");
    if (regex_match(title, str_expr))
        return true;
    return false;
}

// parse post request body "title=xxx&prompt=xxx"
bool request_handler_text_gen::parse_post_body(std::string body, std::map<std::string, std::string>& token_map)
{
    try {
        std::vector<std::string> token_vector;
        boost::split(token_vector, body, boost::is_any_of("&"));
        if ( !token_vector.empty()) {
            for (const std::string& token : token_vector) {
                std::vector<std::string> key_value_pair;
                boost::split(key_value_pair, token, boost::is_any_of("="));
                // e.g. [title, xxx]
                if (key_value_pair.size() == 2) {
                    token_map.insert(std::make_pair(key_value_pair[0], key_value_pair[1]));
                }
            }
            if (token_map.find("title") == token_map.end() || token_map.find("prompt") == token_map.end())
                return false;
            return true;
        }
        return false;
    } catch (...) {
        WARNING << get_client_ip() << ": cannot parse the post request body with request body: " << body;
        return false;
    }

}

bool request_handler_text_gen::handle_post_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    std::string input = request.body();
    std::map<std::string, std::string> token_map;
    if ( !(parse_post_body(input, token_map))) // cannot parse the post request body
    {
        prepare_bad_request_response(response);
        response.body() = post_format_msg; // error msg to user
        response.prepare_payload();
        return false;
    }

    std::string title = token_map["title"];
    std::string text_prompt = token_map["prompt"];

    std::string raw_output;
    std::string output;

    // either text prompt or title does not match the required regex
    if ( (!check_prompts_validity(text_prompt)) || (!check_title_validity(title)))
    {
        prepare_bad_request_response(response);
        response.body() = valid_input_msg; // error msg to user
        response.prepare_payload();
        return false;
    }

    bool file_system_error = false;
    // get a text paragraph from the third-party api, and parse the JSON raw output
    if (curl_api(text_prompt, raw_output))
    {
        if (parse_raw_output(raw_output, output)) {
            response.result(http::status::ok);
            response.body() = output; 
            response.set(http::field::content_type, "text/plain");
            response.prepare_payload();
            // save the entry (title, text prompt and text generated) as JSON to filesystem
            std::string folder = get_dir() + "/" + dot2underscore(get_client_ip());
            if (create_dir(folder))
            {
                std::string path = folder + "/" + title;
                json j;
                j["output"] = output;
                j["text_prompt"] = text_prompt;
                j["title"] = title;
                std::string entry = j.dump(4);
                if (write_to_file(path, entry))
                    return true;
                else {
                    WARNING << get_client_ip() << ": cannot store user's input with title: " << title;
                    file_system_error = true;
                }
            } else {
                WARNING << get_client_ip() << ": cannot create dir";
                file_system_error = true;
            }
        }
    }

    prepare_internal_server_error_response(response);
    response.body() = file_system_error ? file_system_msg : api_communication_msg;// error msg to user
    response.prepare_payload();
	return false;
}

// get file name list for a given ip's prompt history
bool request_handler_text_gen::get_ip_file_list(std::string path, std::string suffix, http::response<http::string_body>& response)
{
    if (boost::filesystem::exists(path)) {// check folder existence
        if (boost::filesystem::is_directory(path)) 
        {
            std::vector<std::string> title_list;// file name list
            for (const boost::filesystem::directory_entry & subentry : boost::filesystem::directory_iterator(path))
            {
                //for every file (eg. /text_gen/127_0_0_0/dummy)
                if (boost::filesystem::is_regular_file(subentry.status())) 
                {
                    title_list.push_back(subentry.path().filename().string());
                }
            }
            std::string split_title_list = boost::algorithm::join(title_list, ",");// convert list[a,b,c,d] to string "a,b,c,d"

            response.result(http::status::ok);
            response.set(http::field::content_type, "application/json");
            json j;
            j["title list"] =  "[" + split_title_list + "]";
            j["IP"] = dot2underscore(get_client_ip());
            response.body() = j.dump(4);
            response.prepare_payload();
            INFO << get_client_ip() << ": sending list of titles for GET request at " << path;
            return true;
        }
    }
    prepare_not_found_response(response);
    WARNING << get_client_ip() << ": cannot find folder for " << path << "\n";
    return false;
}

// read/remove file content from path/suffix
bool request_handler_text_gen::get_title_file(std::string path, std::string suffix, http::response<http::string_body>& response, bool remove)
{
    if (boost::filesystem::exists(path)) {
        if (boost::filesystem::is_directory(path)) {
            std::string titlepath = path + "/" + suffix;
            if(boost::filesystem::is_regular_file(titlepath))// check file exists
            {
                if(remove)// remove the file
                {
                    boost::filesystem::remove(titlepath);
                    INFO << get_client_ip() << ": removing: " << titlepath;
                    response.result(http::status::ok);
                    json j;
                    j["status"] = "removed";
                    j["title"] =  suffix;
                    j["IP"] = dot2underscore(get_client_ip());
                    response.body() = j.dump(4);
                    response.set(http::field::content_type, "application/json");
                    response.prepare_payload();
                    return true;
                }
                else// read the content
                {
                    std::string filecontent;
                    if(read_from_file(titlepath, filecontent))
                    {
                        INFO << "Read from: " << titlepath;
                        response.result(http::status::ok);
                        json j;
                        json filecontent_json = json::parse(filecontent);
                        j["content"] = filecontent_json;
                        j["status"] = "readed";
                        j["title"] =  suffix;
                        j["IP"] = dot2underscore(get_client_ip());
                        response.body() = j.dump(4);
                        response.set(http::field::content_type, "application/json");
                        response.prepare_payload();
                        return true;
                    }
                    else
                    {
                        prepare_not_found_response(response);
                        WARNING << get_client_ip() << ": cannot read from file for " << titlepath << "\n";
                        return false;
                    }
                }
            }
            else
            {
                prepare_not_found_response(response);
                WARNING << get_client_ip() << ": cannot open file for " << titlepath << "\n";
                return false;
            }
        }
    }
    prepare_not_found_response(response);
    WARNING << get_client_ip() << ": cannot find folder for " << path << "\n";
    return false;
}

bool request_handler_text_gen::handle_get_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    std::string suffix;
    // replace "%20" with " "
    std::string url = std::string(request.target());
    std::regex space("%20");
    url = std::regex_replace(url, space, " ");
    if (! check_request_url(url, suffix, true))
    {
        prepare_bad_request_response(response);
        return false;
    }
    std::string path = get_dir() + "/" + dot2underscore(get_client_ip());
    bool result;

    // if url is like /text_gen, return the html to server user's post request
    if (suffix == "")
    {
        path = "../static/static1/text.html";
        std::string content;
        if( read_from_file(path, content)) {
            response.result(http::status::ok);
            response.set(http::field::content_type, "text/html");
            response.body() = content;
            response.prepare_payload();
            return true;
        } else {
            prepare_internal_server_error_response(response);
            return false;
        }

    } 
    // if suffix is history?, we return all the file name for given ip address
    else if (suffix == "history?")
    {
        result =  get_ip_file_list(path, suffix, response);
    }
    // suffix is not empty, we have (eg url = /txt_gen/dummy), we return the content for that file.
    else
    {
        result = get_title_file(path, suffix, response, false);
    }
    return result;

}

bool request_handler_text_gen::handle_delete_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    std::string suffix;
    if (! check_request_url(std::string(request.target()), suffix, false))
    {
        INFO << get_client_ip() << ": URL: " << std::string(request.target()) <<  "not valid";
        prepare_bad_request_response(response);
        response.body() = invalid_url_msg;
        response.prepare_payload();
        return false;
    }
    std::string path = get_dir() + "/" + dot2underscore(get_client_ip());
    return get_title_file(path, suffix, response, true); // set delete flag to true.

}

// for front-end purpose
bool request_handler_text_gen::handle_options_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    response.result(http::status::no_content);
    response.set("Access-Control-Allow-Origin", "*");
    response.set("Access-Control-Allow-Methods", "*");
    response.set("Access-Control-Allow-Headers", "*");
    return true;
}

status request_handler_text_gen::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{	
	std::string target = std::string(request.target());
	INFO << get_client_ip() << ": Using text generation request handler\n";

    auto method = request.method();

    switch (method) {
        // post, get, delete
        case http::verb::post: {
            return handle_post_request(request, response);
        }
        case http::verb::get: {
            return handle_get_request(request, response);
        }
        case http::verb::delete_: {
            return handle_delete_request(request, response);;
        }
        case http::verb::options: {
            // This case is used for CORS preflight
            return handle_options_request(request, response);
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