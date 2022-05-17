#include "request_handler_crud.h"
#include "log.h"
#include <fstream>
#include <stdio.h>
#include <boost/filesystem.hpp>

status request_handler_crud::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    INFO << get_client_ip() << ": Using CRUD API request handler\n";

    auto method = request.method();
    switch (method) {
        // post, get, put, delete
        case http::verb::post: {
            std::string prefix = get_prefix() + "/";
            size_t pos = request.target().find(prefix);
            //if the given location is found, url starts with crud location,
            //and the string of location is smaller than the url, this is a valid path
            if (pos != std::string::npos && pos == 0 && request.target().length() > prefix.length()) {
                std::string key = std::string(request.target().substr(prefix.length()));
                std::string path = get_dir() + "/" + key;
                int value;
                //remove trailing slash
                while(path.length() >= 1 && path[path.length()-1] == '/') {
                    path.pop_back();
                }
                //if entity does not exist yet
                if (file_to_id_.find(key) == file_to_id_.end()) {
                    try {
                        boost::filesystem::create_directory(path);
                    } 
                    catch (const boost::filesystem::filesystem_error& e) {
                        INFO << get_client_ip() << "Unable to create directory for " << key << "\n";
                        //unprocessable entity
                        response.result(http::status::unprocessable_entity);
                        response.set(http::field::content_type, "text/html");
                        response.body() = "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>";
                        response.prepare_payload();
                        return false;
                    }
                    std::vector<int> values = {1};
                    file_to_id_[key] = values;
                    value = 1;
                    INFO << get_client_ip() << "Created directory for new entity: " << key << "\n";
                }
                //if entity already exits
                else {
                    int lowest_available_id = 1;
                    //finds the lowest available id
                    for(int i=0; i<file_to_id_[key].size(); i++) {
                        if (file_to_id_[key][i] == lowest_available_id) {
                            lowest_available_id++;
                        }
                    }
                    file_to_id_[key].push_back(lowest_available_id);
                    std::vector<int> values_copy = file_to_id_[key];
                    std::sort(values_copy.begin(), values_copy.end());
                    file_to_id_[key] = values_copy;
                    value = lowest_available_id;
                }
                //write to file of the given entity value pair
                std::ostringstream oss;     
                oss << request.body();
                std::string body = oss.str();
                oss.clear();
                std::ofstream file(path + "/" + std::to_string(value));
                file << body;
                file.close();
                INFO << get_client_ip() << "Created id " << std::to_string(value) << " for entity " << key << "\n";
                //prepare response
                response.result(http::status::created);
                response.set(http::field::content_type, "text/plain");
                response.body() = "Created entry at {\"id\":" + std::to_string(value) + "}" + "\n";
                response.prepare_payload();
                return true;
            }    
            // cannot find the folder
            response.result(http::status::not_found);
            response.set(http::field::content_type, "text/html");
            response.body() = not_found_msg;
            response.prepare_payload();

            return false;
        }
        case http::verb::get:
            // TODO: handle GET request, (R)ead an entity or list all if body empty
            // response.result(http::status::ok), etc.
            break;
        case http::verb::put: {
            std::string prefix = get_prefix() + "/";
            size_t pos = request.target().find(prefix);
            //if the given location is found, url starts with crud location,
            //and the string of location is smaller than the url, this is a valid path
            if (pos != std::string::npos && pos == 0 && request.target().length() > prefix.length()) {
                std::string key = std::string(request.target().substr(prefix.length()));
                int value;
                //remove trailing slash
                while(key.length() >= 1 && key[key.length()-1] == '/') {
                    key.pop_back();
                }
                std::size_t found = key.find_last_of("/");
                std::string id_name = key.substr(found+1);
                std::string file_name = key.substr(0, found);
                std::string::const_iterator it = id_name.begin();
                while (it != id_name.end() && std::isdigit(*it)) {
                    ++it;
                }
                //if after last / is integer, this is a valid path
                if(!id_name.empty() && it == id_name.end()) {
                    //if key before last / exists
                    value = std::stoi(id_name);
                    if (file_to_id_.find(file_name) != file_to_id_.end()) {
                        std::vector<int> id_names = file_to_id_[file_name]; 
                        if (std::find(id_names.begin(), id_names.end(), value) == id_names.end()) {
                            id_names.push_back(value);
                            std::vector<int> values_copy = id_names;
                            std::sort(values_copy.begin(), values_copy.end());
                            file_to_id_[file_name] = values_copy;
                            response.result(http::status::created);
                            response.body() = "Created entry at {\"id\":" + std::to_string(value) + "}" + "\n";
                            INFO << get_client_ip() << "Created id " << std::to_string(value) << " for entity " << file_name << "\n";
                        }
                        else {
                            response.result(http::status::ok);
                            response.body() = "Updated entry at {\"id\":" + std::to_string(value) + "}" + "\n";
                            INFO << get_client_ip() << "File updated for id " << std::to_string(value) << " for entity " << file_name << "\n";
                        }
                    }
                    //if key before last / does not exist
                    else {
                        try {
                            boost::filesystem::create_directory(get_dir() + '/' + file_name);
                        } 
                        catch (const boost::filesystem::filesystem_error& e) {
                            INFO << get_client_ip() << "Unable to create directory for " << file_name << "\n";
                            //unprocessable entity
                            response.result(http::status::unprocessable_entity);
                            response.set(http::field::content_type, "text/html");
                            response.body() = "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>";
                            response.prepare_payload();
                            return false;
                        }
                        std::vector<int> values = {value};
                        file_to_id_[file_name] = values;
                        response.result(http::status::created);
                        response.body() = "Created entry at {\"id\":" + std::to_string(value) + "}" + "\n";
                        INFO << get_client_ip() << "Created directory for new entity: " << file_name << " and created id " << std::to_string(value) << "\n";
                    }  
                    std::string path = get_dir() + "/" + key;
                    std::ostringstream oss;     
                    oss << request.body();
                    std::string body = oss.str();
                    oss.clear();
                    //ofstream automatically overwrites, which is expected for PUT
                    std::ofstream file(path);
                    file << body;
                    file.close();
                    response.prepare_payload();
                    //prepare response
                    return true;
                }
                else {
                    //else error bad request
                    response.result(http::status::bad_request);
                    response.set(http::field::content_type, "text/html");
                    response.body() = "<html><head><title>Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>";
                    response.prepare_payload();
                  
                    return false;
                }
            }    
            // cannot find the folder
            response.result(http::status::not_found);
            response.set(http::field::content_type, "text/html");
            response.body() = not_found_msg;
            response.prepare_payload();

            return false;
        }
        case http::verb::delete_: {
            std::string prefix = get_prefix() + "/";
            std::string target = std::string(request.target().substr(prefix.length()));
            std::string path_str = get_dir() + "/" + target;
            if (boost::filesystem::is_directory(path_str)) {
                response.result(http::status::bad_request);
                response.set(http::field::content_type, "text/plain");
                response.body() = "Invalid format; expected a file ID.\n";
                response.prepare_payload();
                WARNING << "Warning: CRUD DELETE operations must specify a file ID";
                return false;
            }

            std::size_t found = target.find_last_of("/");
            std::string key = target.substr(0,found);
            std::string value_str = target.substr(found+1,target.length());
            int value;
            try {
                value = std::stoi(value_str);
            } catch (const std::exception&) {
                response.result(http::status::bad_request);
                response.set(http::field::content_type, "text/plain");
                response.body() = "Invalid format; expected a file ID.\n";
                response.prepare_payload();
                WARNING << "Warning: CRUD DELETE operations must specify a file ID";
                return false;
            }

            INFO << "Incoming CRUD request to delete file: " << "/" << target;
            const char* path = path_str.c_str();
            if (boost::filesystem::exists(path_str)) {
                if (file_to_id_.find(key) != file_to_id_.end()) {
                    if (std::remove(path) == 0) {
                        // Remove value from file:id mapping
                        file_to_id_[key].erase(std::remove(file_to_id_[key].begin(), file_to_id_[key].end(), value), file_to_id_[key].end());
                        response.result(http::status::ok);
                        response.set(http::field::content_type, "text/plain");
                        response.body() = "Successfully deleted file at /" + target + "." + "\n";
                        response.prepare_payload();
                        INFO << "Successfully deleted file at /" << target;
                        return true;
                    }
                    else {
                        response.result(http::status::internal_server_error);
                        response.set(http::field::content_type, "text/plain");
                        response.body() = "Error deleting file at /" + target + "." + "\n";
                        ERROR << "CRUD request to delete file at /" << target << " failed";
                    }
                }
                else {
                    response.result(http::status::internal_server_error);
                    response.set(http::field::content_type, "text/plain");
                    response.body() = "Could not find file ID for /" + target + "." + "\n";
                    ERROR << "CRUD request to delete file at /" << target << " failed; no file ID";
                }
            }
            else {
                response.result(http::status::not_found);
                response.set(http::field::content_type, "text/plain");
                response.body() = "No file found at /" + key + "." + "\n";
                ERROR << "File not found.";
            }
            response.prepare_payload();

            return false;
            break;
        }
        default:
            // TODO: send 404/400/something, we didn't get a valid CRUD method
            response.result(http::status::method_not_allowed);
            return false;
    }

    // Temp code, just to test that the handler is actually being called.
    // Remember to delete for finished product.
    response.result(http::status::ok);
    response.set(http::field::content_type, "text/html");
    response.body() = "<!DOCTYPE html>"
        "<html>"
            "<head>"
                "<title>CRUD success!</title>"
            "</head>"
            "<body>"
                "<h1>Congrats!</h1>"
                "<p>The CRUD handler was successfully created and called.</p>"
            "</body>"
        "</html>";
    response.prepare_payload();

    return true;
}
