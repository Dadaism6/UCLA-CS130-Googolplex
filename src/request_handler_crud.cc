#include "request_handler_crud.h"
#include "log.h"
#include <fstream>
#include <stdio.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

bool request_handler_crud::check_request_url(std::string url, std::string& key) {
    std::string prefix = get_prefix() + "/";
    size_t pos = url.find(prefix);
    if (pos != std::string::npos && pos == 0 && url.length() > prefix.length()) {
        key = std::string(url.substr(prefix.length()));
        //remove trailing slash
        while(key.length() >= 1 && key[key.length()-1] == '/') {
            key.pop_back();
        }
        return true;
    }
    key = "";
    return false;
}

int request_handler_crud::get_next_id(std::string key) {
    int lowest_available_id = 1;
    //finds the lowest available id
    if (file_to_id_.find(key) == file_to_id_.end()) {
        std::vector<int> values = {1};
        file_to_id_[key] = values;
        return lowest_available_id;
    }

    for(int i=0; i<file_to_id_[key].size(); i++) {
        if (file_to_id_[key][i] == lowest_available_id) {
            lowest_available_id++;
        }
    }
    insert_to_map(key, lowest_available_id);
    return lowest_available_id;
}

bool request_handler_crud::insert_to_map(std::string key, int id) {
    if (file_to_id_.find(key) != file_to_id_.end()) {
        std::vector<int> id_names = file_to_id_[key]; 
        if (std::find(id_names.begin(), id_names.end(), id) == id_names.end()) {
            file_to_id_[key].push_back(id);
            std::vector<int> values_copy = file_to_id_[key];
            std::sort(values_copy.begin(), values_copy.end());
            file_to_id_[key] = values_copy;
            return true;
        }
    }
    return false;
}

void request_handler_crud::prepare_created_response(int value, std::string entity, http::response<http::string_body>& response) {
    response.result(http::status::created);
    response.set(http::field::content_type, "text/plain");
    response.body() = "Created entry at {\"id\":" + std::to_string(value) + "}" + "\n";
    response.prepare_payload();
    INFO << get_client_ip() << ": created id " << std::to_string(value) << " for entity " << entity << "\n";
}

void request_handler_crud::prepare_unprocessable_entity_response(std::string dir, http::response<http::string_body>& response)
{
    response.result(http::status::unprocessable_entity);
    response.set(http::field::content_type, "text/html");
    response.body() = "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>";
    response.prepare_payload();
    INFO << get_client_ip() << ": cannot create directory " << dir << "\n";
}

void request_handler_crud::prepare_internal_server_error_response(std::string path, http::response<http::string_body>& response)
{
    response.result(http::status::internal_server_error);
    response.set(http::field::content_type, "text/html");
    response.body() = "<html><head><title>Internal Server Error</title></head><body><h1>500 Internal Server Error</h1></body></html>";
    response.prepare_payload();
    INFO << get_client_ip() << ": cannot perform file operation at " << path << "\n";
}

void request_handler_crud::prepare_bad_request_response(http::response<http::string_body>& response)
{
    response.result(http::status::bad_request);
    response.set(http::field::content_type, "text/plain");
    response.body() = "Invalid CRUD method.\n";
    response.prepare_payload();
}

void request_handler_crud::prepare_not_found_response(http::response<http::string_body>& response)
{
    response.result(http::status::not_found);
    response.set(http::field::content_type, "text/html");
    response.body() = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
    response.prepare_payload();
}

bool request_handler_crud::handle_post_request(std::string suffix, http::request<http::string_body> request, http::response<http::string_body>& response)
{
    std::string path = get_dir() + "/" + suffix;
    //if entity does not exist yet
    if (file_to_id_.find(suffix) == file_to_id_.end()) {
        if ( !create_dir(path)) {
            prepare_unprocessable_entity_response(path, response);
            return false;
        }
    }
    int value = get_next_id(suffix);
    // write to file of the given entity value pair
    if (write_to_file((path + "/" + std::to_string(value)), std::string(request.body())))
    {
        prepare_created_response(value, suffix, response);
        return true;
    }
    prepare_internal_server_error_response(suffix + "/" + std::to_string(value), response);
    return false;
}

bool request_handler_crud::handle_get_request(std::string suffix, http::response<http::string_body>& response)
{
    std::string path = get_dir() + "/" + suffix;
    if (boost::filesystem::exists(path)) {
        if (boost::filesystem::is_directory(path)) {
            std::vector<int> ids = file_to_id_[suffix];
            std::vector<std::string> ids_str;
            // Convert ID vector of type int to type string
            std::transform(ids.begin(), ids.end(), std::back_inserter(ids_str),
                [](const int& id) { return std::to_string(id); });
            std::string id_list = boost::algorithm::join(ids_str, ",");

            response.result(http::status::ok);
            response.set(http::field::content_type, "text/plain");
            response.body() = "[" + id_list + "]\n";
            response.prepare_payload();
            INFO << get_client_ip() << ": sending list of IDs for GET request at " << path;
            return true;
        }
        else {
            std::string content;
            if (read_from_file(path, content)) {
                response.result(http::status::ok);
                response.body() = content;
                response.prepare_payload();
                INFO << get_client_ip() << ": CRUD GET request: Finish Setting Response\n";
                return true;
            }
        }
    }
    prepare_not_found_response(response);
    return false;
}

bool request_handler_crud::handle_put_request(std::string suffix, http::request<http::string_body> request, http::response<http::string_body>& response)
{
    int value;
    std::string path = get_dir() + "/" + suffix;
    std::size_t found = suffix.find_last_of("/");
    std::string id_name = suffix.substr(found+1);
    std::string entity = suffix.substr(0, found);

    std::string::const_iterator it = id_name.begin();
    while (it != id_name.end() && std::isdigit(*it)) {
        ++it;
    }
    //if after last / is integer, this is a valid path
    if(!id_name.empty() && it == id_name.end()) {
        //if key before last / exists
        value = std::stoi(id_name);
        if (file_to_id_.find(entity) != file_to_id_.end()) {
            if ( insert_to_map(entity, value)) {
                prepare_created_response(value, entity, response);
            } else {
                response.result(http::status::ok);
                response.body() = "Updated entry at {\"id\":" + std::to_string(value) + "}" + "\n";
                response.prepare_payload();
                INFO << get_client_ip() << "File updated for id " << std::to_string(value) << " for entity " << entity << "\n";
            }
        }
        //if key before last / does not exist
        else if (! create_dir(get_dir() + '/' + entity))
        {
            prepare_unprocessable_entity_response(get_dir() + '/' + entity, response);
            return false;
        } else {
            std::vector<int> values = {value};
            file_to_id_[entity] = values;
            prepare_created_response(value, entity, response);
        } 

        if (write_to_file(path, std::string(request.body())))  
            return true;
    }
    prepare_not_found_response(response);
    return false;
}

bool request_handler_crud::handle_delete_request(std::string suffix, http::response<http::string_body>& response)
{
    std::string path = get_dir() + "/" + suffix;
    if (boost::filesystem::exists(path)) 
    {
        if (boost::filesystem::is_directory(path)) {
            response.result(http::status::bad_request);
            response.set(http::field::content_type, "text/plain");
            response.body() = "Invalid format; expected a file ID.\n";
            response.prepare_payload();
            WARNING << get_client_ip() << ": CRUD DELETE operations must specify a file ID";
            return false;
        }

        std::size_t found = suffix.find_last_of("/");
        std::string id_name = suffix.substr(found+1);
        std::string entity = suffix.substr(0, found);

        int value;
        try {
            value = std::stoi(id_name);
        } catch (const std::exception&) {
            response.result(http::status::bad_request);
            response.set(http::field::content_type, "text/plain");
            response.body() = "Invalid format; expected a file ID.\n";
            response.prepare_payload();
            WARNING << get_client_ip() << ": CRUD DELETE operations must specify a file ID";
            return false;
        }

        if (file_to_id_.find(entity) != file_to_id_.end()) {
            if (std::remove(path.c_str()) == 0) {
                // Remove value from file:id mapping
                file_to_id_[entity].erase(std::remove(file_to_id_[entity].begin(), file_to_id_[entity].end(), value), file_to_id_[entity].end());
                response.result(http::status::ok);
                response.set(http::field::content_type, "text/plain");
                response.body() = "Successfully deleted file at /" + entity + "." + "\n";
                response.prepare_payload();
                INFO << get_client_ip() << ": successfully deleted file at /" << entity;
                return true;
            }
            else {
                prepare_internal_server_error_response(entity, response);
                return false;
            }
        }
    }
    prepare_not_found_response(response);
    return false;
}

status request_handler_crud::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    INFO << get_client_ip() << ": using CRUD API request handler\n";

    auto method = request.method();

    std::string suffix;

    if (! check_request_url(std::string(request.target()), suffix))
    {
        prepare_bad_request_response(response);
        return false;
    }
    std::string path = get_dir() + "/" + suffix;

    switch (method) {
        // post, get, put, delete
        case http::verb::post: {
            return handle_post_request(suffix, request, response);
        }
        case http::verb::get: {
            return handle_get_request(suffix, response);
        }
        case http::verb::put: {
            return handle_put_request(suffix, request, response);
        }
        case http::verb::delete_: {
            return handle_delete_request(suffix, response);
        }
        default:
        {
            prepare_bad_request_response(response);
            return false;
        }
    }
}
