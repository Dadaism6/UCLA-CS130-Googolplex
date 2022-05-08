#include <map>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "session.h"
#include <memory>

#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "request_handler_not_found.h"
#include "request_handler_factory.h"

using ::testing::AtLeast;    
using ::testing::_;

namespace http = boost::beast::http;

class SessionTest:public::testing::Test
{
    public:
        SessionTest() {
            // manually create dispatcher mapping
            config_arg echo_arg;
            echo_arg.location = "/echo";
            echo_arg.root = "";
            echo_arg.handler_type = "EchoHandler";
            config_arg static_arg;
            static_arg.location = "/static";
            static_arg.root = "";
            static_arg.handler_type = "StaticHandler";
            config_arg not_found_arg;
            not_found_arg.location = "/";
            not_found_arg.root = "";
            not_found_arg.handler_type = "404Handler";
            routes["/echo"] = std::shared_ptr<EchoHandlerFactory>(new EchoHandlerFactory(echo_arg));
            routes["/static"] = std::shared_ptr<StaticHandlerFactory>(new StaticHandlerFactory(static_arg));
            routes["/"] = std::shared_ptr<NotFoundHandlerFactory>(new NotFoundHandlerFactory(not_found_arg));
        }

    protected:
        boost::asio::io_service io_service;
        boost::system::error_code success_ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::system::error_code bad_ec = boost::system::errc::make_error_code(boost::system::errc::connection_refused);

        char request_data_1[1] = "";
        const int request_data_1_invalid_length = -1;
        const int request_data_1_length = 0;
        char request_data_2[5] = "\r\n\r\n";
        const int request_data_2_length = 4;
        const int request_data_2_invalid_length = 1025;
        char request_data_3[40] = "GET / HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        const int request_data_3_length = 39;
        char request_data_4[6] = "hello";
        const int request_data_4_length = 5;
        char echo_request[44] = "GET /echo HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        const int echo_request_length = 43;
        char invalid_url[43] = "GET /ech HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        const int invalid_url_length = 42;
        char static_request[51] = "GET /static/test HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        const int static_request_length = 50;
        char trailing_slash[21] = "GET // HTTP/1.1\r\n\r\n";
        const int trailing_slash_length = 20;

        std::string not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 85\r\n\r\n<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";

        bool status;
        std::string rep_str;
        std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes;
        boost::beast::http::response<boost::beast::http::string_body> rep;
};

// gmock session class
class MockSession: public session {
    public:
        MockSession(boost::asio::io_service& io_service, std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes) : session(io_service, routes) {}
        MOCK_METHOD0(start, void());
        MOCK_METHOD0(read, void());
        MOCK_METHOD0(recycle, void());
};

// mock test handle_write with no error code
TEST_F(SessionTest, MockWrite1) {
    MockSession mocksession(io_service, routes);
    EXPECT_CALL(mocksession, start()).Times(AtLeast(1));
    mocksession.handle_write(success_ec);
} 

// mock test handle_write with error code
TEST_F(SessionTest, MockWrite2) {
    MockSession mocksession(io_service, routes);
    EXPECT_CALL(mocksession, recycle()).Times(AtLeast(1));
    mocksession.handle_write(bad_ec);
} 

// mock test handle_read with no error code
TEST_F(SessionTest, MockRead1) {
    MockSession mocksession(io_service, routes);
    EXPECT_CALL(mocksession, read()).Times(AtLeast(1));
    mocksession.handle_read(success_ec, 0);
} 

// mock test handle_read with error code
TEST_F(SessionTest, MockRead2) {
    MockSession mocksession(io_service, routes);
    EXPECT_CALL(mocksession, recycle()).Times(AtLeast(1));
    mocksession.handle_read(bad_ec, 0);
} 

// test session constructor
TEST_F(SessionTest, SessionConstruction) {
    session s(io_service, routes);
    EXPECT_TRUE(true);
}

// negative data length
TEST_F(SessionTest, GenerateResponse_1) {
    session s(io_service, routes);
    rep = s.generate_response(request_data_1, request_data_1_invalid_length);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// out of bound data length
TEST_F(SessionTest, GenerateResponse_2) {
    session s(io_service, routes);
    rep = s.generate_response(request_data_2, request_data_2_invalid_length);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// null printer as input
TEST_F(SessionTest, GenerateResponse_3) {
    session s(io_service, routes);
    rep = s.generate_response(nullptr, 0);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// empty data not valid request
TEST_F(SessionTest, GenerateResponse_4) {
    session s(io_service, routes);
    rep = s.generate_response(request_data_1, request_data_1_length);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// only crlf*2 not valid
TEST_F(SessionTest, GenerateResponse_5) {
    session s(io_service, routes);
    rep = s.generate_response(request_data_2, request_data_2_length);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// normal invalid http request
TEST_F(SessionTest, GenerateResponse_6) {
    session s(io_service, routes);
    rep = s.generate_response(request_data_4, request_data_4_length);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// not found handler request
TEST_F(SessionTest, GetReply) {
    session s(io_service, routes);
    rep_str = s.get_reply(request_data_3, request_data_3_length);
    EXPECT_EQ(rep_str, not_found);
}

// echo request
TEST_F(SessionTest, EchoRequest) {
    std::string expected_content(echo_request);
    session s(io_service, routes);
    rep = s.generate_response(echo_request, echo_request_length);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(rep.body(), expected_content);
}

// invalid uri
TEST_F(SessionTest, InvalidURL) {
    session s(io_service, routes);
    rep = s.generate_response(invalid_url, invalid_url_length);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// static request
TEST_F(SessionTest, StaticRequest) {
    session s(io_service, routes);
    rep = s.generate_response(static_request, static_request_length);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// not found request
TEST_F(SessionTest, NotFoundRequest) {
    session s(io_service, routes);
    rep = s.generate_response(request_data_3, request_data_3_length);
    EXPECT_EQ(rep.result(), http::status::not_found);
    rep = s.generate_response(trailing_slash, trailing_slash_length);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// test recycle and delete
TEST_F(SessionTest, SessionRecycle) {
    session* s = new session(io_service, routes);
    s -> recycle();
    ASSERT_DEATH({s -> recycle();}, "");
    EXPECT_TRUE(true);
}
