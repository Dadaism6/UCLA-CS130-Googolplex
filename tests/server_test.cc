#include <map>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "server.h"
#include "gmock/gmock.h"

using ::testing::AtLeast;
using ::testing::_;

class MockSessionReal : public session {
  public:
    MockSessionReal(boost::asio::io_service& io_service, std::map<std::string, std::string> addrmap) : session(io_service, addrmap) {}
    MOCK_METHOD2(handle_read, bool(const boost::system::error_code&,
			size_t));
    MOCK_METHOD1(handle_write, bool(const boost::system::error_code&));
    MOCK_METHOD0(start, void());
};

class ServerTest:public::testing::Test
{
    protected:
        boost::asio::io_service io_service;
        boost::system::error_code success_ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::system::error_code bad_ec = boost::system::errc::make_error_code(boost::system::errc::connection_refused);
        short port = 8080;
        bool status;
        std::map<std::string, std::string> addrmap;
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
    server s(io_service,port,addrmap);
    EXPECT_TRUE(true);
}

// server mock test using a mock session
TEST_F(ServerTest, MockServerTest1) {
    MockSessionReal m_session(io_service, addrmap);
    EXPECT_CALL(m_session, start()).Times(AtLeast(1));
    server m_server(io_service, port, addrmap);
    MockSessionReal* session_ptr = &m_session;
    m_server.handle_accept(session_ptr, success_ec);
}

// server handle accept with error code (bad)
// cannot use gmock here, may raise child aborted
// use self defined mock here
TEST_F(ServerTest, HandleAccept_2) {
    MockServer s(io_service, port, addrmap);
    MockSession* mock_session = new MockSession(io_service, addrmap);
    status = s.handle_accept(mock_session, bad_ec);
    EXPECT_TRUE(status);
}
