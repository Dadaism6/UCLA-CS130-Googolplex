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
#include <boost/bind.hpp>
#include <boost/asio.hpp>


#include "server.h"
#include "session.h"
#include "config_parser.h"
#include "log.h"
using boost::asio::ip::tcp;


void init()
{
	logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");
	logging::add_common_attributes();
	std::string format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%";
    logging::add_file_log
    (
        keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
		keywords::auto_flush = true,
		keywords::format = format
    );
	logging::add_console_log
	(
		std::cout,
		keywords::format = format
	);
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::trace
    );
}

int main(int argc, char* argv[])
{ 
	// Initialize logger
	init();
    src::severity_logger< logging::trivial::severity_level > lg;
	// BOOST_LOG_SEV(lg, trace) << "A trace severity message";
    // BOOST_LOG_SEV(lg, debug) << "A debug severity message";
    // BOOST_LOG_SEV(lg, info) << "An informational severity message";
    // BOOST_LOG_SEV(lg, warning) << "A warning severity message";
    // BOOST_LOG_SEV(lg, error) << "An error severity message";
    // BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";
	try
	{
		if (argc != 2) {
			BOOST_LOG_SEV(lg, logging::trivial::error) << "Usage: webserver <config_file>\n";
			std::cerr << "Usage: webserver <config_file>\n";
			
			return 1;
		}
		int port = -1;
		BOOST_LOG_SEV(lg, logging::trivial::info) << "Preparing to parse the config file\n";
		NginxConfigParser config_parser;
		NginxConfig config;
		config_parser.Parse(argv[1], &config, &port);
		boost::asio::io_service io_service;
		server s(io_service,port);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;  
}
