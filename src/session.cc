#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "session.h"

using boost::asio::ip::tcp;


session::session(boost::asio::io_service& io_service)
: socket_(io_service)
{
  next_out_data_fill_pos = header_length;
  memset(last4bytes_, 0, 5);
  strcpy(response_header_, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
}

tcp::socket& session::socket()
{
  return socket_;
}

void session::start()
{
  socket_.async_read_some(boost::asio::buffer(in_data_, max_length),
    boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}


void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
  if (!error)
  {
    strncpy(out_data_ + next_out_data_fill_pos, in_data_, bytes_transferred);
    next_out_data_fill_pos += bytes_transferred;
    if (strlen(out_data_ + header_length) >= 4)
      strncpy(last4bytes_, out_data_ + next_out_data_fill_pos - 4, 4);
    else 
      memset(last4bytes_, 0, 5);
    if (!strcmp(last4bytes_, "\r\n\r\n")) { // done reading
      strncpy(out_data_, response_header_, header_length);
      boost::asio::async_write(socket_,
          boost::asio::buffer(out_data_, strlen(out_data_)),
          boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
    }
    else {
      memset(in_data_, 0, max_length);
      socket_.async_read_some(boost::asio::buffer(in_data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
  }
  else
  {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
    memset(out_data_, 0, max_length + 46); // reset out buffer
    memset(response_header_, 0, header_length + 1); // reset response header
    next_out_data_fill_pos = header_length;
    strcpy(response_header_, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
    socket_.async_read_some(boost::asio::buffer(in_data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    delete this;
  }
}

