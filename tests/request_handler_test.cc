#include "gtest/gtest.h"
#include "config_arg.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "request_handler_not_found.h"
#include "request_handler_health.h"
#include "request_handler_block.h"

#include <fstream>
#include <stdio.h>

namespace http = boost::beast::http;

class RequestHandlerTest:public::testing::Test
{
  public:
    RequestHandlerTest() {
        echo_arg.location = "/";

        static_arg.location = "/" + static_;
        static_arg.root = base_dir;

        not_found_arg.location = "/";

        health_arg.location = "/health";

        block_arg.location = "/sleep";

        req_handler_echo = new request_handler_echo(echo_arg.location, echo_arg.root);
        req_handler_static = new request_handler_static(static_arg.location, static_arg.root);
        req_handler_not_found = new request_handler_not_found(not_found_arg.location, not_found_arg.root);
        req_handler_health = new request_handler_health(health_arg.location, health_arg.root);
        req_handler_block = new request_handler_block(health_arg.location, block_arg.root);
    }

    ~RequestHandlerTest() {
        delete req_handler_echo;
        delete req_handler_static;
        delete req_handler_not_found;
        delete req_handler_health;
        delete req_handler_block;
    }
  protected:
    void write_to_file(std::string path);

    http::request<http::string_body> req;
    http::response<http::string_body> rep;
    enum {content_length_field = 0, content_type_field = 1};

    std::string base_dir = ".";
    std::string static_ = "static";
    std::string file_name_no_extension = "test";
    std::string file_name;

    config_arg echo_arg;
    config_arg static_arg;
    config_arg not_found_arg;
    config_arg health_arg;
    config_arg block_arg;

    request_handler* req_handler_echo;
    request_handler* req_handler_static;
    request_handler* req_handler_not_found;
    request_handler* req_handler_health;
    request_handler* req_handler_block;
};

void RequestHandlerTest::write_to_file(std::string path)
{
    std::ofstream file(path);
    file << "test";
    file.close();
}

TEST_F(RequestHandlerTest, EchoTest)
{
    req_handler_echo -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), " HTTP/1.1\r\n\r\n");
}

TEST_F(RequestHandlerTest, inValidStaticTest_1)
{
    req.method(http::verb::get);
    req.target("/statIc");
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

TEST_F(RequestHandlerTest, inValidStaticTest_2)
{
    req.target("/hello/" + static_ + "/");
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

TEST_F(RequestHandlerTest, inValidStaticTest_3)
{
    req.target( "/" + static_ + "/");
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

TEST_F(RequestHandlerTest, FileNotFoundStaticTest)
{
    req.target( "/" + static_ + "/not_a_file");
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_1)
{
    file_name = file_name_no_extension + ".txt";
    req.target( "/" + static_ + "/" + file_name + "/");
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "text/plain");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_2)
{
    file_name = file_name_no_extension + ".html";
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "text/html");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_3)
{
    file_name = file_name_no_extension + ".htm";
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "text/html");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_4)
{
    file_name = file_name_no_extension + ".jpg";
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "image/jpeg");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_5)
{
    file_name = file_name_no_extension + ".jpeg";
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "image/jpeg");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_6)
{
    file_name = file_name_no_extension + ".png";
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "image/png");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_7)
{
    file_name = file_name_no_extension + ".gif";
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "image/gif");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_8)
{
    file_name = file_name_no_extension + ".zip";
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "application/zip");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, ContentTypeStaticTest_9)
{
    file_name = file_name_no_extension;
    req.target( "/" + static_ + "/" + file_name);
    write_to_file(file_name);
    req_handler_static -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    EXPECT_EQ(rep.base()[http::field::content_type], "text/plain");
    std::remove(file_name.c_str());
}

TEST_F(RequestHandlerTest, NotFoundTest)
{
    req_handler_not_found -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

TEST_F(RequestHandlerTest, HealthTest)
{
    req.target( "/health");
    req_handler_health -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "OK");
}

TEST_F(RequestHandlerTest, BlockTest)
{
    req.target( "/sleep/2");
    req_handler_block -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "Sleep 2s!\n");
}