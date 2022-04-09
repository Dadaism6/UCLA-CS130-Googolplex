#ifndef SESSION_H
#define SESSION_H

#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "http/request.h"
#include "http/request_parser.h"
#include "http/reply.h"

using boost::asio::ip::tcp;

class session
{
public:
  session(boost::asio::io_service& io_service);

  tcp::socket& socket();

  void start();

private:
  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);

  void handle_write(const boost::system::error_code& error);
  
  tcp::socket socket_;
  int next_out_data_fill_pos;
  char last4bytes_[5];
  enum { max_length = 1024 };
  enum { header_length = 45 };
  char in_data_[max_length];
  char out_data_[max_length + header_length]; // header length is 45
  char response_header_ [header_length + 1];
  
};

#endif  // SESSION_H