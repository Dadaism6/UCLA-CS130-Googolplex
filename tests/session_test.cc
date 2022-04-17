#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "session.h"
#include "http/reply.h"

class SessionTest:public::testing::Test
{
    protected:
        boost::asio::io_service io_service;
        char request_data_1[1] = "";
        char request_data_2[5] = "\r\n\r\n";
        char request_data_3[40] = "GET / HTTP/1.1\r\nHost: Zhengtong Liu\r\n\r\n";
        char request_data_4[6] = "hello";
        bool status;
        http::server::reply rep;
};

// attempt for mock test
class MockSession: public session {
    public:
        boost::asio::io_service io_service;
        MockSession() : session(io_service) {}
        // MOCK_METHOD(void, start, (), (override));
        MOCK_METHOD(void, handle_read, (const boost::system::error_code&, size_t), (override));
        MOCK_METHOD(void, handle_write, (const boost::system::error_code&), (override));
};

// test session constructor
TEST_F(SessionTest, SessionConstruction) {
    session s(io_service);
    EXPECT_TRUE(true);
}

// attempt for mock test
// TEST_F(SessionTest, SessionStart) {
//     MockSession* mock_session = new MockSession();
//     mock_session -> start();
//     EXPECT_TRUE(true);
//     delete mock_session;
// }

// negative data length
TEST_F(SessionTest, ParseRequest_1) {
    session s(io_service);
    status = s.parse_request(request_data_1, -1);
    EXPECT_FALSE(status);
}

// out of bound data length
TEST_F(SessionTest, ParseRequest_2) {
    session s(io_service);
    status = s.parse_request(request_data_2, 1025);
    EXPECT_FALSE(status);
}

// null printer as input
TEST_F(SessionTest, ParseRequest_3) {
    session s(io_service);
    status = s.parse_request(nullptr, 0);
    EXPECT_FALSE(status);
}

// empty data not valid request
TEST_F(SessionTest, ParseRequest_4) {
    session s(io_service);
    status = s.parse_request(request_data_1, 0);
    EXPECT_FALSE(status);
}

// only crlf*2 not valid
TEST_F(SessionTest, ParseRequest_5) {
    session s(io_service);
    status = s.parse_request(request_data_2, 4);
    EXPECT_FALSE(status);
}

// valid http request
TEST_F(SessionTest, ParseRequest_6) {
    session s(io_service);
    status = s.parse_request(request_data_3, 39);
    EXPECT_TRUE(status);
}

// data length can be longer than actual request
TEST_F(SessionTest, ParseRequest_7) {
    session s(io_service);
    status = s.parse_request(request_data_3, 42);
    EXPECT_TRUE(status);
}

// data length cannot be shorter
TEST_F(SessionTest, ParseRequest_8) {
    session s(io_service);
    status = s.parse_request(request_data_3, 38);
    EXPECT_FALSE(status);
}

// normal invalid http request
TEST_F(SessionTest, ParseRequest_9) {
    session s(io_service);
    status = s.parse_request(request_data_4, 5);
    EXPECT_FALSE(status);
}

// null pointer + negative data length
TEST_F(SessionTest, AddHeader_1) {
    session s(io_service);
    rep = s.add_header(nullptr, -1);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// invalid request + out of bound data length
TEST_F(SessionTest, AddHeader_2) {
    session s(io_service);
    rep = s.add_header(request_data_1, 1025);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}

// test the length of null pointer
TEST_F(SessionTest, AddHeader_3) {
    session s(io_service);
    rep = s.add_header(nullptr, 0);
    ASSERT_EQ(rep.status, http::server::reply::bad_request);
    EXPECT_EQ(rep.headers[0].value, "0");
}

// test when strlen(data) != data length passed in
TEST_F(SessionTest, AddHeader_4) {
    session s(io_service);
    rep = s.add_header(request_data_4, 3);
    ASSERT_EQ(rep.status, http::server::reply::bad_request);
    EXPECT_EQ(rep.headers[0].value, "5");
}

// valid request, status should be 200 OK
TEST_F(SessionTest, AddHeader_5) {
    session s(io_service);
    rep = s.add_header(request_data_3, 39);
    EXPECT_EQ(rep.status, http::server::reply::ok);
}

// test length of valid request
TEST_F(SessionTest, AddHeader_6) {
    session s(io_service);
    rep = s.add_header(request_data_3, 39);
    ASSERT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[0].value, "39");
}

// valid test, strlen(data) < data length
TEST_F(SessionTest, AddHeader_7) {
    session s(io_service);
    rep = s.add_header(request_data_3, 43);
    ASSERT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[0].value, "39");
}

// valid test, strlen(data) > data length (no longer valid)
TEST_F(SessionTest, AddHeader_8) {
    session s(io_service);
    rep = s.add_header(request_data_3, 38);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
    EXPECT_EQ(rep.headers[0].value, "39");
}

// test content type
TEST_F(SessionTest, AddHeader_9) {
    session s(io_service);
    rep = s.add_header(request_data_3, 39);
    ASSERT_EQ(rep.status, http::server::reply::ok);
    EXPECT_EQ(rep.headers[1].value, "text/plain");
}

// valid request, data length out of bound
TEST_F(SessionTest, AddHeader_10) {
    session s(io_service);
    rep = s.add_header(request_data_3, 1025);
    EXPECT_EQ(rep.status, http::server::reply::bad_request);
}