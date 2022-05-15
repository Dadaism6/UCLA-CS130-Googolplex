#include "server.h"
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
    INFO << "Server shuts down" << "\n";
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

bool server::create_dispatcher(std::map<std::string, config_arg> addrmap) 
{
    for (auto const& mapping : addrmap) {
        config_arg curr_handler = mapping.second;
        std::string handler_type = mapping.second.handler_type;
        if (handler_type == "StaticHandler")
            routes[mapping.first] = std::shared_ptr<StaticHandlerFactory>(new StaticHandlerFactory(curr_handler));
        else if (handler_type == "EchoHandler")
            routes[mapping.first] = std::shared_ptr<EchoHandlerFactory>(new EchoHandlerFactory(curr_handler));
        else if (handler_type == "CrudHandler")
            routes[mapping.first] = std::shared_ptr<CrudHandlerFactory>(new CrudHandlerFactory(curr_handler));
        else
            routes[mapping.first] = std::shared_ptr<NotFoundHandlerFactory>(new NotFoundHandlerFactory(curr_handler));
    }
    return true;
}
