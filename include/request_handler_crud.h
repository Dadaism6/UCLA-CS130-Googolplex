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
};

#endif // REQUEST_HANDLER_CRUD_H
