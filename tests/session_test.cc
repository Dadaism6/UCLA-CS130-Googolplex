#include <map>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "session.h"
#include "http/reply.h"

using ::testing::AtLeast;    
using ::testing::_;

class SessionTest:public::testing::Test
{
    protected:
        boost::asio::io_service io_service;
        char request_data_1[1] = "";
        char request_data_2[5] = "\r\n\r\n";
        char request_data_3[40] = "GET / HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char request_data_4[6] = "hello";
        char echo_request[44] = "GET /echo HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char invalid_url[43] = "GET /ech HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char static_request[51] = "GET /static/test HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        std::map<std::string, config_arg> addrmap;
        bool status;
        http::server::reply rep;
        boost::system::error_code success_ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::system::error_code bad_ec = boost::system::errc::make_error_code(boost::system::errc::connection_refused);
};

// gmock session class
class MockSession: public session {
    public:
        MockSession(boost::asio::io_service& io_service, std::map<std::string, config_arg> addrmap) : session(io_service, addrmap) {}
        MOCK_METHOD0(start, void());
        MOCK_METHOD0(read, void());
        MOCK_METHOD0(recycle, void());
};

// mock test handle_write with no error code
TEST_F(SessionTest, MockWrite1) {
    MockSession mocksession(io_service, addrmap);
    EXPECT_CALL(mocksession, start()).Times(AtLeast(1));
    mocksession.handle_write(success_ec);
} 

// mock test handle_write with error code
TEST_F(SessionTest, MockWrite2) {
    MockSession mocksession(io_service, addrmap);
    EXPECT_CALL(mocksession, recycle()).Times(AtLeast(1));
    mocksession.handle_write(bad_ec);
} 

// mock test handle_read with no error code
TEST_F(SessionTest, MockRead1) {
    MockSession mocksession(io_service, addrmap);
    EXPECT_CALL(mocksession, read()).Times(AtLeast(1));
    mocksession.handle_read(success_ec, 0);
} 

// mock test handle_read with error code
TEST_F(SessionTest, MockRead2) {
    MockSession mocksession(io_service, addrmap);
    EXPECT_CALL(mocksession, recycle()).Times(AtLeast(1));
    mocksession.handle_read(bad_ec, 0);
} 

// test session constructor
TEST_F(SessionTest, SessionConstruction) {
    session s(io_service, addrmap);
    EXPECT_TRUE(true);
}

// negative data length
TEST_F(SessionTest, ParseRequest_1) {
    session s(io_service, addrmap);
    rep = s.get_reply(request_data_1, -1);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// out of bound data length
TEST_F(SessionTest, ParseRequest_2) {
    session s(io_service, addrmap);
    rep = s.get_reply(request_data_2, 1025);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// null printer as input
TEST_F(SessionTest, ParseRequest_3) {
    session s(io_service, addrmap);
    rep = s.get_reply(nullptr, 0);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// empty data not valid request
TEST_F(SessionTest, ParseRequest_4) {
    session s(io_service, addrmap);
    rep = s.get_reply(request_data_1, 0);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// only crlf*2 not valid
TEST_F(SessionTest, ParseRequest_5) {
    session s(io_service, addrmap);
    rep = s.get_reply(request_data_2, 4);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// valid http request, uri is "/"
TEST_F(SessionTest, ParseRequest_6) {
    config_arg args;
    args.handler_type = "EchoHandler";
    args.location = "/";
    addrmap["/"] = args;
    session s(io_service, addrmap);
    rep = s.get_reply(request_data_3, 39);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    addrmap.erase("/");
}

// normal invalid http request
TEST_F(SessionTest, ParseRequest_7) {
    session s(io_service, addrmap);
    rep = s.get_reply(request_data_4, 5);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// echo request
TEST_F(SessionTest, EchoRequest) {
    config_arg args;
    args.handler_type = "EchoHandler";
    args.location = "/echo";
    addrmap["/echo"] = args;

    std::string expected_content(echo_request);
    session s(io_service, addrmap);
    rep = s.get_reply(echo_request, 43);
    EXPECT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.content, expected_content);

    addrmap.erase("/echo");
}

// invalid uri
TEST_F(SessionTest, InvalidURL) {
    session s(io_service, addrmap);
    rep = s.get_reply(invalid_url, 42);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// static request
TEST_F(SessionTest, StaticRequest) {
    config_arg args;
    args.handler_type = "StaticHandler";
    args.location = "/static";
    args.root = "dummy";
    addrmap["/static"] = args;
    session s(io_service, addrmap);
    rep = s.get_reply(static_request, 50);
    EXPECT_EQ(rep.status, http::server::reply::not_found);
}

// not found request
TEST_F(SessionTest, NotFoundRequest) {
    config_arg args;
    args.handler_type = "404Handler";
    args.location = "/";
    args.root = "";
    addrmap["/"] = args;
    session s(io_service, addrmap);
    rep = s.get_reply(request_data_3, 39);
    EXPECT_EQ(rep.status, http::server::reply::not_found);
}

// test recycle and delete
TEST_F(SessionTest, SessionRecycle) {
    session* s = new session(io_service, addrmap);
    s -> recycle();
    ASSERT_DEATH({s -> recycle();}, "");
    EXPECT_TRUE(true);
}
