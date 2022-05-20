#include "server.h"
#include "log.h"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

using boost::asio::ip::tcp;


server::server(boost::asio::io_service& io_service, short port, std::map<std::string, config_arg> addrmap)
: io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    create_dispatcher(addrmap);
    start_accept();
    INFO << "Start the server with concurrency " << std::to_string(boost::thread::hardware_concurrency()) << " on port " << port << "\n";
}

server::~server()
{
    stop();
    INFO << "Server shuts down" << "\n";
}

void server::start(int n_threads) {
    // Create a pool of threads to run all of the io_services.
    std::vector<boost::shared_ptr<boost::thread>> threads;
    for (std::size_t i = 0; i < n_threads; ++i)
    {
        boost::shared_ptr<boost::thread> thread(new boost::thread(
            boost::bind(&boost::asio::io_service::run, &io_service_)));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
        threads[i]->join();
}

void server::stop()
{
    io_service_.stop();
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
        else if (handler_type == "BlockHandler")
            routes[mapping.first] = std::shared_ptr<BlockHandlerFactory>(new BlockHandlerFactory(curr_handler));
            else if (handler_type == "HealthHandler")
            routes[mapping.first] = std::shared_ptr<HealthHandlerFactory>(new HealthHandlerFactory(curr_handler));
        else
            routes[mapping.first] = std::shared_ptr<NotFoundHandlerFactory>(new NotFoundHandlerFactory(curr_handler));
    }
    return true;
}
