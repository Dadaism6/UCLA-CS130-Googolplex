#include <map>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "session.h"

#include "request_handler.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "request_handler_not_found.h"

using ::testing::AtLeast;    
using ::testing::_;

namespace http = boost::beast::http;

class SessionTest:public::testing::Test
{
    public:
        SessionTest() {
            dispatcher["/echo"] = new request_handler_echo("/echo", "");
            dispatcher["/static"] = new request_handler_static("/static", "dummy");
            dispatcher["/"] = new request_handler_not_found("/", "");
        }

        ~SessionTest() {
            for (auto const& x : dispatcher) {
                if (x.second != NULL)
                    delete x.second;
            }
        }

    protected:
        boost::asio::io_service io_service;
        boost::system::error_code success_ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::system::error_code bad_ec = boost::system::errc::make_error_code(boost::system::errc::connection_refused);

        char request_data_1[1] = "";
        char request_data_2[5] = "\r\n\r\n";
        char request_data_3[40] = "GET / HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char request_data_4[6] = "hello";
        char echo_request[44] = "GET /echo HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char invalid_url[43] = "GET /ech HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char static_request[51] = "GET /static/test HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char trailing_slash[21] = "GET // HTTP/1.1\r\n\r\n";

        std::string not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 85\r\n\r\n<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";

        bool status;
        std::string rep_str;
        std::map<std::string, request_handler*> dispatcher;
        boost::beast::http::response<boost::beast::http::string_body> rep;
};

// gmock session class
class MockSession: public session {
    public:
        MockSession(boost::asio::io_service& io_service, std::map<std::string, request_handler*> dispatcher) : session(io_service, dispatcher) {}
        MOCK_METHOD0(start, void());
        MOCK_METHOD0(read, void());
        MOCK_METHOD0(recycle, void());
};

// mock test handle_write with no error code
TEST_F(SessionTest, MockWrite1) {
    MockSession mocksession(io_service, dispatcher);
    EXPECT_CALL(mocksession, start()).Times(AtLeast(1));
    mocksession.handle_write(success_ec);
} 

// mock test handle_write with error code
TEST_F(SessionTest, MockWrite2) {
    MockSession mocksession(io_service, dispatcher);
    EXPECT_CALL(mocksession, recycle()).Times(AtLeast(1));
    mocksession.handle_write(bad_ec);
} 

// mock test handle_read with no error code
TEST_F(SessionTest, MockRead1) {
    MockSession mocksession(io_service, dispatcher);
    EXPECT_CALL(mocksession, read()).Times(AtLeast(1));
    mocksession.handle_read(success_ec, 0);
} 

// mock test handle_read with error code
TEST_F(SessionTest, MockRead2) {
    MockSession mocksession(io_service, dispatcher);
    EXPECT_CALL(mocksession, recycle()).Times(AtLeast(1));
    mocksession.handle_read(bad_ec, 0);
} 

// test session constructor
TEST_F(SessionTest, SessionConstruction) {
    session s(io_service, dispatcher);
    EXPECT_TRUE(true);
}

// negative data length
TEST_F(SessionTest, GenerateResponse_1) {
    session s(io_service, dispatcher);
    rep = s.generate_response(request_data_1, -1);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// out of bound data length
TEST_F(SessionTest, GenerateResponse_2) {
    session s(io_service, dispatcher);
    rep = s.generate_response(request_data_2, 1025);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// null printer as input
TEST_F(SessionTest, GenerateResponse_3) {
    session s(io_service, dispatcher);
    rep = s.generate_response(nullptr, 0);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// empty data not valid request
TEST_F(SessionTest, GenerateResponse_4) {
    session s(io_service, dispatcher);
    rep = s.generate_response(request_data_1, 0);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// only crlf*2 not valid
TEST_F(SessionTest, GenerateResponse_5) {
    session s(io_service, dispatcher);
    rep = s.generate_response(request_data_2, 4);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// normal invalid http request
TEST_F(SessionTest, GenerateResponse_6) {
    session s(io_service, dispatcher);
    rep = s.generate_response(request_data_4, 5);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

TEST_F(SessionTest, GetReply) {
    session s(io_service, dispatcher);
    rep_str = s.get_reply(request_data_3, 39);
    EXPECT_EQ(rep_str, not_found);
}

// echo request
TEST_F(SessionTest, EchoRequest) {
    std::string expected_content(echo_request);
    session s(io_service, dispatcher);
    rep = s.generate_response(echo_request, 43);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(rep.body(), expected_content);
}

// invalid uri
TEST_F(SessionTest, InvalidURL) {
    session s(io_service, dispatcher);
    rep = s.generate_response(invalid_url, 42);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// static request
TEST_F(SessionTest, StaticRequest) {
    session s(io_service, dispatcher);
    rep = s.generate_response(static_request, 50);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// not found request
TEST_F(SessionTest, NotFoundRequest) {
    session s(io_service, dispatcher);
    rep = s.generate_response(request_data_3, 39);
    EXPECT_EQ(rep.result(), http::status::not_found);
    rep = s.generate_response(trailing_slash, 20);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// test recycle and delete
TEST_F(SessionTest, SessionRecycle) {
    session* s = new session(io_service, dispatcher);
    s -> recycle();
    ASSERT_DEATH({s -> recycle();}, "");
    EXPECT_TRUE(true);
}
