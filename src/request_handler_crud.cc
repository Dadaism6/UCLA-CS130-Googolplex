#include "request_handler_crud.h"
#include "log.h"

status request_handler_crud::handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
{
    INFO << get_client_ip() << ": Using CRUD API request handler\n";

    auto method = request.method();
    switch (method) {
        // post, get, put, delete
        case http::verb::post:
            // TODO: handle POST request, (C)reate new entity
            // response.result(http::status::created), etc.
            break;
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
