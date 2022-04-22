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

#include "server.h"
#include "session.h"
#include "config_parser.h"
#include "log.h"

using boost::asio::ip::tcp;

// signal handler
void signalHandler( int signum ) {
   INFO << "Interrupt signal received. Shutting Down server. \n";
   exit(signum);  
}

int main(int argc, char* argv[])
{ 
	INFO << "Server Startup\n";

	// handle Ctrl-C
	signal(SIGINT, signalHandler); 

	try
	{
		if (argc != 2) {
			ERROR << "Usage: webserver <config_file>\n";			
			return 1;
		}
		int port = -1;
		std::string basepath = "";
		INFO << "Preparing to parse the config file\n";
		NginxConfigParser config_parser;
		NginxConfig config;
		config_parser.Parse(argv[1], &config, &port, &basepath);
		boost::asio::io_service io_service;
		INFO << "Finish parsing, prepare to start the server\n";
		server s(io_service,port, basepath);
		io_service.run();
	}
	catch (std::exception& e)
	{
		ERROR << "Exception: " << e.what() << "\n";
	}
	return 0;  
}
