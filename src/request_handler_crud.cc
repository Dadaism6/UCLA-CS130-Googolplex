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
                    std::vector<int> values = {1};
                    file_to_id_[key] = values;
                    value = 1;
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
                response.set(http::field::content_type, "text/html");
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
            break;
        }
        case http::verb::get:
            // TODO: handle GET request, (R)ead an entity or list all if body empty
            // response.result(http::status::ok), etc.
            break;
        case http::verb::put:
            // TODO: handle PUT request, (U)pdate existing entity's data
            // status OK for update of existing entity, Created for new one, etc.
            break;
        case http::verb::delete_:
            // TODO: handle DELETE request, (D)elete entity from db
            break;
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
