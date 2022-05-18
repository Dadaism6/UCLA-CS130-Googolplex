#ifndef REQUEST_HANDLER_CRUD_H
#define REQUEST_HANDLER_CRUD_H

#include "request_handler.h"

namespace http = boost::beast::http;

/**
 * Handles calls to the CRUD API.
 * Database is implemented as a folder w/ file names corresponding to IDs.
 * Data folder location is specified with 'data_path' arg in config file, and stored as the dir_ member.
 * So if you get a GET request at /api/Shoes/1, the data will be at 'get_dir()/Shoes/1'.
 */
class request_handler_crud : public request_handler
{
  public:
    request_handler_crud(std::string location, std::string root, std::map<std::string, std::vector<int>>& file_to_id) 
      : request_handler(location, root), file_to_id_(file_to_id) {}
    status handle_request(http::request<http::string_body> request, http::response<http::string_body>& response);
  private:
    std::map<std::string, std::vector<int>>& file_to_id_;
    bool check_request_url(std::string url, std::string& key);
    int get_next_id(std::string key);
    bool write_to_file(std::string path, std::string content);
    bool create_dir(std::string path);
    bool read_from_file(std::string path, std::string& content);
    bool insert_to_map(std::string key, int id);
    void prepare_created_response(int value, std::string entity, http::response<http::string_body>& response);
    void prepare_unprocessable_entity_response(std::string dir, http::response<http::string_body>& response);
    void prepare_bad_request_response(http::response<http::string_body>& response);
    void prepare_not_found_response(http::response<http::string_body>& response);
    bool handle_post_request(std::string suffix, http::request<http::string_body> request, http::response<http::string_body>& response);
    bool handle_get_request(std::string suffix, http::response<http::string_body>& response);
    bool handle_put_request(std::string suffix, http::request<http::string_body> request, http::response<http::string_body>& response);
    bool handle_delete_request(std::string suffix, http::response<http::string_body>& response);
};

#endif // REQUEST_HANDLER_CRUD_H
