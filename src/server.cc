#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "server.h"
#include "request_handler.h"
#include "log.h"
using boost::asio::ip::tcp;


server::server(boost::asio::io_service& io_service, short port, std::map<std::string, config_arg> addrmap)
: io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    create_dispatcher(addrmap);
    start_accept();
    INFO << "Start the server on port " << port << "\n";
}

server::~server()
{
    INFO << "Server shut down perform clean up" << "\n";
    for (auto const& route : routes) {
        delete (route.second);
    }
    INFO << "Server clean up completes" << "\n";
}

void server::start_accept()
{
    session* new_session = new session(io_service_, routes);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
}

bool server::handle_accept(session* new_session,
    const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->start();
    }
    else
    {
        delete new_session;
        new_session = NULL;
    }

    start_accept();
    return (new_session == nullptr);
}

void server::create_dispatcher(std::map<std::string, config_arg> addrmap) 
{
    for (auto const& mapping : addrmap) {
        routes[mapping.first] = new RequestHandlerFactory(mapping.second);
    }
}
