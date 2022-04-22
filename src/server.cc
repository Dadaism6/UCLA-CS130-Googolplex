#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "server.h"
#include "log.h"
using boost::asio::ip::tcp;


server::server(boost::asio::io_service& io_service, short port, std::string basepath)
: io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    basepath(basepath)
{
    start_accept();
    INFO << "Start the server, port:" << port << " and basepath: " << basepath << "\n";
}

void server::start_accept()
{
    session* new_session = new session(io_service_, basepath);
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


