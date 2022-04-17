#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "server.h"

using ::testing::AtLeast;

class ServerTest:public::testing::Test
{
    protected:
        boost::asio::io_service io_service;
        boost::system::error_code success_ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::system::error_code bad_ec = boost::system::errc::make_error_code(boost::system::errc::connection_refused);
        short port = 8080;
        bool status;
};

class MockServer: public server {
    public:
        using server::server;
        void start_accept() {std::cout << "start_accept: dummy function here" << std::endl; }
};

class MockSession: public session {
    public:
        using session::session;
        void start() {std::cout << "start: dummy function here" << std::endl; }
        void recycle() {std::cout << "recycle: dummy function here" << std::endl; }
        void read() {std::cout << "read: dummy function here" << std::endl; }
};

// test server construction
TEST_F(ServerTest, ServerConstruction) {
    server s(io_service,port);
    EXPECT_TRUE(true);
}

// server handle accept with error code (sucess)
TEST_F(ServerTest, HandleAccept_1) {
    MockServer s(io_service, port);
    MockSession* mock_session = new MockSession(io_service);
    status = s.handle_accept(mock_session, success_ec);
    EXPECT_FALSE(status);
    delete mock_session;
}

// server handle accept with error code (bad)
TEST_F(ServerTest, HandleAccept_2) {
    MockServer s(io_service, port);
    MockSession* mock_session = new MockSession(io_service);
    status = s.handle_accept(mock_session, bad_ec);
    EXPECT_TRUE(status);
}