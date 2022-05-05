//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <cstdlib>
#include <iostream>
#include <csignal>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <map>
#include "server.h"
#include "session.h"
#include "config_parser.h"
#include "log.h"

using boost::asio::ip::tcp;

// signal handler
void signalHandler( int signum ) {
   INFO << "Interrupt signal \"" << strsignal(signum) << "\" received. Shutting Down server. \n";
   exit(signum);  
}

int main(int argc, char* argv[])
{ 
	INFO << "Server Startup\n";

	// handle Ctrl-C
	signal(SIGINT, signalHandler); 
	//handle timeout
	signal(SIGALRM, signalHandler);
	// handle process termination
	signal(SIGTERM, signalHandler);

	try
	{
		if (argc != 2) {
			FATAL << "Usage: webserver <config_file>\n";			
			return 1;
		}
		int port = -1;
		std::map<std::string, config_arg>  addrmap;
		INFO << "Preparing to parse the config file\n";
		NginxConfigParser config_parser;
		NginxConfig config;
		
		bool valid_handler = true;
		config_parser.Parse(argv[1], &config, &port, &addrmap);

		config_arg default_arg;
		std::string error_msg = "";

		for (auto const& x : addrmap) {
			if (default_arg.valid_handler_types.find(x.second.handler_type) == default_arg.valid_handler_types.end())
			{
				valid_handler = false;
				error_msg += "handler type " + x.second.handler_type + " invalid; ";
			}
		}

		if ( !valid_handler)
		{
			error_msg += "Server only accepts the handler types: ";
			for (auto const& x: default_arg.valid_handler_types)
				error_msg += x + " ";
			error_msg += "\n";
			FATAL << error_msg;
			return 1;
		}

		boost::asio::io_service io_service;
		INFO << "Finish parsing, prepare to start the server\n";
		server s(io_service,port, addrmap);
		io_service.run();
	}
	catch (std::exception& e)
	{
		ERROR << "Exception: " << e.what() << "\n";
	}
	return 0;  
}
