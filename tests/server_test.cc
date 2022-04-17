#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "server.h"

using ::testing::AtLeast;

class ServerTest:public::testing::Test
{
    protected:
        boost::asio::io_service io_service;
        short port = 8080;
};

// attempt for mock test
class MockServer : public server {
    public:
        boost::asio::io_service io_service;
        short port = 8080;
        MockServer() : server(io_service, port) {}
        MOCK_METHOD0(handle_accept, void());

};

TEST_F(ServerTest, ServerConstruction) {
    server s(io_service,port);
    EXPECT_TRUE(true);
}

// TEST_F(ServerTest, HandleAccept) {
//     MockServer mock_server;
//     EXPECT_CALL(mock_server, handle_accept()).Times(AtLeast(1));
// }