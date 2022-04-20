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

#include <boost/phoenix.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>


#include "server.h"
#include "session.h"
#include "config_parser.h"

using boost::asio::ip::tcp;

namespace logging = boost::log;
namespace attrs = logging::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(process_id, "ProcessID", attrs::current_process_id::value_type )

attrs::current_process_id::value_type::native_type get_native_process_id(
	logging::value_ref<attrs::current_process_id::value_type,
	tag::process_id> const& pid)
{
    if (pid > 0)
        return pid->native_id();
    return 0;
}

void init()
{
    logging::add_file_log
    (
        keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
		keywords::format =
        (
            expr::stream
				<< "[" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
                << "][Process " << boost::phoenix::bind(&get_native_process_id, process_id.or_none()) << "]: " 
				<< "<" << logging::trivial::severity << "> " << expr::smessage
        )                            /*< log record format >*/
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
    logging::add_common_attributes();
    using namespace logging::trivial;
    src::severity_logger< severity_level > lg;

	BOOST_LOG_SEV(lg, trace) << "A trace severity message";
    BOOST_LOG_SEV(lg, debug) << "A debug severity message";
    BOOST_LOG_SEV(lg, info) << "An informational severity message";
    BOOST_LOG_SEV(lg, warning) << "A warning severity message";
    BOOST_LOG_SEV(lg, error) << "An error severity message";
    BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";


	try
	{
		if (argc != 2) {
			std::cerr << "Usage: webserver <config_file>\n";
			return 1;
		}
		int port = -1;
		NginxConfigParser config_parser;
		NginxConfig config;
		config_parser.Parse(argv[1], &config, &port);
		boost::asio::io_service io_service;
		using namespace std; // For atoi.
		server s(io_service,port);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;  
}
