#include <iostream>
#include <fstream>
#include <stdio.h>
#include "gtest/gtest.h"
#include "http/request.h"
#include "http/reply.h"
#include "request_handler.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"

class RequestHandlerTest:public::testing::Test
{
  protected:
    void write_to_file(std::string path);
    request_handler* req_handler;
    bool valid = true;
    bool not_valid = false;
    http::server::request req;
    http::server::reply rep;
    enum {content_length_field = 0, content_type_field = 1};
    std::string base_dir = ".";
    std::string static_ = "static";
    std::string file_name_no_extension = "test";
    std::string file_name;
};

void RequestHandlerTest::write_to_file(std::string path)
{
    std::ofstream file(path);
    file << "test";
    file.close();
}

TEST_F(RequestHandlerTest, InValidEchoTest)
{
    req_handler = new request_handler_echo(req, not_valid);
    rep = req_handler -> handle_request(NULL, "");
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
    delete req_handler;
}

TEST_F(RequestHandlerTest, ValidEmptyEchoTest)
{
    req_handler = new request_handler_echo(req, valid);
    rep = req_handler -> handle_request(NULL, "");
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_length_field].value, "0");
    delete req_handler;
}

TEST_F(RequestHandlerTest, ValidRequestEchoTest)
{
    char request_data[40] = "GET / HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
    req_handler = new request_handler_echo(req, valid);
    rep = req_handler -> handle_request(request_data, "");
    EXPECT_EQ(rep.status, http::server::reply::ok);

    std::string string_req_data(request_data);
    EXPECT_EQ(rep.content, request_data);
    delete req_handler;
}

TEST_F(RequestHandlerTest, inValidStaticTest_1)
{
    req.uri = "/statIc";
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, "");
    EXPECT_EQ(rep.status, http::server::reply::not_found);
    delete req_handler;
}

TEST_F(RequestHandlerTest, inValidStaticTest_2)
{
    req.uri = "/hello/" + static_ + "/";
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, "");
    EXPECT_EQ(rep.status, http::server::reply::not_found);
    delete req_handler;
}

TEST_F(RequestHandlerTest, inValidStaticTest_3)
{
    req.uri = "/" + static_ + "/";
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, "");
    EXPECT_EQ(rep.status, http::server::reply::not_found);
    delete req_handler;
}

TEST_F(RequestHandlerTest, FileNotFoundStaticTest)
{
    req.uri = "/" + static_ + "/not_a_file";
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, "");
    EXPECT_EQ(rep.status, http::server::reply::not_found);
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_1)
{
    file_name = file_name_no_extension + ".txt";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.content, "test");
    EXPECT_EQ(rep.headers[content_type_field].value, "text/plain");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_2)
{
    file_name = file_name_no_extension + ".html";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "text/html");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_3)
{
    file_name = file_name_no_extension + ".htm";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "text/html");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_4)
{
    file_name = file_name_no_extension + ".jpg";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "image/jpeg");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_5)
{
    file_name = file_name_no_extension + ".jpeg";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "image/jpeg");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_6)
{
    file_name = file_name_no_extension + ".png";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "image/png");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_7)
{
    file_name = file_name_no_extension + ".gif";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "image/gif");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_8)
{
    file_name = file_name_no_extension + ".zip";
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "application/zip");
    std::remove(file_name.c_str());
    delete req_handler;
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_9)
{
    file_name = file_name_no_extension;
    req.uri = "/" + static_ + "/" + file_name;
    write_to_file(file_name);
    req_handler = new request_handler_static(req, valid);
    rep = req_handler -> handle_request(NULL, base_dir);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[content_type_field].value, "text/plain");
    std::remove(file_name.c_str());
    delete req_handler;
}