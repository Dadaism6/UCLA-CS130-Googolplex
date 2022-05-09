#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "server.h"
#include "request_handler_factory.h"
#include "request_handler_not_found.h"

#include <map>
#include <memory>

using ::testing::AtLeast;
using ::testing::_;

class MockSessionReal : public session {
  public:
    MockSessionReal(boost::asio::io_service& io_service, std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes) : session(io_service, routes) {}
    MOCK_METHOD2(handle_read, bool(const boost::system::error_code&,
			size_t));
    MOCK_METHOD1(handle_write, bool(const boost::system::error_code&));
    MOCK_METHOD0(start, void());
};

class ServerTest:public::testing::Test
{
    public:
        ServerTest() {
            config_arg test_arg;
            test_arg.location = "/";
            test_arg.root = "";
            routes["/"] = std::shared_ptr<NotFoundHandlerFactory>(new NotFoundHandlerFactory(test_arg));

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
            addrmap_factory["/echo"] = echo_arg;
            addrmap_factory["/static"] = static_arg;
            addrmap_factory["/"] = not_found_arg;
        }

    protected:
        boost::asio::io_service io_service;
        boost::system::error_code success_ec = boost::system::errc::make_error_code(boost::system::errc::success);
        boost::system::error_code bad_ec = boost::system::errc::make_error_code(boost::system::errc::connection_refused);
        short port = 8080;
        bool status;
        std::map<std::string, config_arg> addrmap;
        std::map<std::string, config_arg> addrmap_factory;
        std::map<std::string, std::shared_ptr<RequestHandlerFactory>> routes;
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
    server s(io_service, port, addrmap);
    EXPECT_TRUE(true);
}

// server mock test using a mock session
TEST_F(ServerTest, MockServerTest1) {
    MockSessionReal m_session(io_service, routes);
    EXPECT_CALL(m_session, start()).Times(AtLeast(1));
    server m_server(io_service, port, addrmap);
    MockSessionReal* session_ptr = &m_session;
    m_server.handle_accept(session_ptr, success_ec);
}

/* server handle accept with error code (bad)
   cannot use gmock here, may raise child aborted
   use self defined mock here */
TEST_F(ServerTest, HandleAccept_2) {
    MockServer s(io_service, port, addrmap);
    MockSession* mock_session = new MockSession(io_service, routes);
    status = s.handle_accept(mock_session, bad_ec);
    EXPECT_TRUE(status);
}

// Test dispatcher
TEST_F(ServerTest, Factory) {
    MockServer s(io_service, port, addrmap_factory);
    status = s.create_dispatcher(addrmap_factory);
    EXPECT_TRUE(status);
}
