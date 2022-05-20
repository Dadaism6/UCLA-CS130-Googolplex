#ifndef SERVER_H
#define SERVER_H

#include "session.h"
#include "request_handler.h"
#include "config_arg.h"

#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <map>
#include <memory>

using boost::asio::ip::tcp;

class server
{
	public:
		server(boost::asio::io_service& io_service, short port, std::map<std::string, config_arg> addrmap);
		~server();
		virtual void start_accept();
		bool handle_accept(session* new_session, const boost::system::error_code& error);
		bool create_dispatcher(std::map<std::string, config_arg> addrmap);

		//Run the server's io_service loop.
		void start(int n_threads);
		// Stop the server.
		void stop();

	private:
		boost::asio::io_service& io_service_;
		tcp::acceptor acceptor_;
		std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes;
};

#endif  // SERVER_H