#include <map>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "server.h"
#include "gmock/gmock.h"

#include "request_handler.h"
#include "request_handler_not_found.h"

using ::testing::AtLeast;
using ::testing::_;

class MockSessionReal : public session {
  public:
    MockSessionReal(boost::asio::io_service& io_service, std::map<std::string, request_handler*> dispatcher) : session(io_service, dispatcher) {}
    MOCK_METHOD2(handle_read, bool(const boost::system::error_code&,
			size_t));
    MOCK_METHOD1(handle_write, bool(const boost::system::error_code&));
    MOCK_METHOD0(start, void());
};

class ServerTest:public::testing::Test
{
    public:
        ServerTest() {
            dispatcher["/"] = new request_handler_not_found("/", "");
        }
        ~ServerTest() {
            for (auto const& x : dispatcher) {
                if (x.second != NULL)
                    delete x.second;
            }
        }
    protected:
        boost::asio::io_service io_service;
        boost::system::error_code success_ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::system::error_code bad_ec = boost::system::errc::make_error_code(boost::system::errc::connection_refused);
        short port = 8080;
        bool status;
        std::map<std::string, request_handler*> dispatcher;
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
    server s(io_service, port, dispatcher);
    EXPECT_TRUE(true);
}

// server mock test using a mock session
TEST_F(ServerTest, MockServerTest1) {
    MockSessionReal m_session(io_service, dispatcher);
    EXPECT_CALL(m_session, start()).Times(AtLeast(1));
    server m_server(io_service, port, dispatcher);
    MockSessionReal* session_ptr = &m_session;
    m_server.handle_accept(session_ptr, success_ec);
}

// server handle accept with error code (bad)
// cannot use gmock here, may raise child aborted
// use self defined mock here
TEST_F(ServerTest, HandleAccept_2) {
    MockServer s(io_service, port, dispatcher);
    MockSession* mock_session = new MockSession(io_service, dispatcher);
    status = s.handle_accept(mock_session, bad_ec);
    EXPECT_TRUE(status);
}
