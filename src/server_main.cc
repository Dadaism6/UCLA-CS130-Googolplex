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
using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{ 
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
