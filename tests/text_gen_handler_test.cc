#include "gtest/gtest.h"
#include "config_arg.h"
#include "request_handler_text_gen.h"

#include <fstream>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

class TextGenHandlerTest :public::testing::Test
{
    public:
        TextGenHandlerTest() {
            text_gen_arg.root = "../text_gen";
            text_gen_arg.location = "/text_gen";
            text_gen_arg.api_key = "c97c056c-bb68-4ee5-82b6-97bd2e47c1b6";
            req_handler_text_gen = new request_handler_text_gen(text_gen_arg.location, text_gen_arg.root, text_gen_arg.api_key);
        }
    
        ~TextGenHandlerTest() {
            delete req_handler_text_gen;
        }
    
        std::string not_found = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
        http::request<http::string_body> req;
        http::response<http::string_body> rep;

        config_arg text_gen_arg;
        request_handler* req_handler_text_gen;
};

// invalid method
TEST_F(TextGenHandlerTest, InvalidMethodTest)
{
    req.target( text_gen_arg.location );
    req.method( http::verb::trace );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// empty post request body
TEST_F(TextGenHandlerTest, PostInvalidTest1)
{
    req.target( text_gen_arg.location );
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// cannot create directory
TEST_F(TextGenHandlerTest, PostInvalidTest2)
{
    config_arg bad_text_gen_arg;
    bad_text_gen_arg.root = "./text_gen1/text_gen1";
    bad_text_gen_arg.location = "/text_gen_bad";
    bad_text_gen_arg.api_key = "c97c056c-bb68-4ee5-82b6-97bd2e47c1b6";
    request_handler* bad_req_handler_text_gen = new request_handler_text_gen(bad_text_gen_arg.location, bad_text_gen_arg.root, bad_text_gen_arg.api_key);
    req.target( bad_text_gen_arg.location );
    req.body() = "title=hello&prompt=hello";
    req.method( http::verb::post );
    bad_req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::internal_server_error);
}

// valid post request
TEST_F(TextGenHandlerTest, PostValidTest1)
{
    req.target( text_gen_arg.location );
    req.body() = "title=hello&prompt=hello";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
}

// another valid post request, with space in the body
TEST_F(TextGenHandlerTest, PostValidTest2)
{
    req.target( text_gen_arg.location );
    req.body() = "title=hi hi&prompt=this is me";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
}

// invalid title in post request body (has "?")
TEST_F(TextGenHandlerTest, PostInvalidTitleTest1)
{
    req.target( text_gen_arg.location );
    req.body() = "title=hello?&prompt=hello";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// invalid title in post request body (empty title)
TEST_F(TextGenHandlerTest, PostInvalidTitleTest2)
{
    req.target( text_gen_arg.location );
    req.body() = "title=&prompt=hello";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// invalid prompt in post request body (has '@')
TEST_F(TextGenHandlerTest, PostInvalidPromptTest1)
{
    req.target( text_gen_arg.location );
    req.body() = "title=hello&prompt=hello@";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// invalid prompt in post request body (empty prompt)
TEST_F(TextGenHandlerTest, PostInvalidPromptTest2)
{
    req.target( text_gen_arg.location );
    req.body() = "title=hello&prompt=";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// invalid post request body (no prompt)
TEST_F(TextGenHandlerTest, PostInvalidBodyTest1)
{
    req.target( text_gen_arg.location );
    req.body() = "title=hello";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// invalid post request body (split with '=' -> three strings)
TEST_F(TextGenHandlerTest, PostInvalidBodyTest2)
{
    req.target( text_gen_arg.location );
    req.body() = "title=hello=1";
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// invalid get request, invalid URL
TEST_F(TextGenHandlerTest, GetInvalidTest1)
{
    req.target("/haha");
    req.method( http::verb::get );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// invalid get request, invalid URL, not start with "/"
TEST_F(TextGenHandlerTest, GetInvalidTest2)
{
    req.target("haha");
    req.method( http::verb::get );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// valid get request, entry that does not exist
TEST_F(TextGenHandlerTest, GetValidTest1)
{
    req.target( (text_gen_arg.location + "/haha") );
    req.method( http::verb::get );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// valid get request, entry exists
TEST_F(TextGenHandlerTest, GetValidTest2)
{
    req.target( (text_gen_arg.location + "/hello") );
    req.method( http::verb::get );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
}

// valid get request, entry exists, has "%20" to be replaced with space
TEST_F(TextGenHandlerTest, GetValidTest3)
{
    req.target( (text_gen_arg.location + "/hi%20hi") );
    req.method( http::verb::get );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
}

// valid get request, with no entry specified in URL
TEST_F(TextGenHandlerTest, GetValidTest4)
{
    req.target( text_gen_arg.location );
    req.method( http::verb::get );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
}

// valid get request, query the history
TEST_F(TextGenHandlerTest, GetValidTest5)
{
    req.target( (text_gen_arg.location + "/history?") );
    req.method( http::verb::get );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
}

// invalid delete request, invalid URL
TEST_F(TextGenHandlerTest, DeleteInvalidTest)
{
    req.target("dummy");
    req.method( http::verb::delete_ );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

// valid delete request, delete an entry that exists
TEST_F(TextGenHandlerTest, DeleteValidTest1)
{
    req.target( (text_gen_arg.location + "/hi hi") );
    req.method( http::verb::delete_ );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
}

// valid delete request, delete the only entry that exists
TEST_F(TextGenHandlerTest, DeleteValidTest2)
{
    req.target( (text_gen_arg.location + "/hello") );
    req.method( http::verb::delete_ );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    boost::filesystem::remove_all(text_gen_arg.location);
    EXPECT_TRUE(true);
}

// valid delete request, try to delete an entry that does not exist
TEST_F(TextGenHandlerTest, DeleteValidTest3)
{
    req.target( (text_gen_arg.location + "/not_a_file") );
    req.method( http::verb::delete_ );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

// valid options request
TEST_F(TextGenHandlerTest, OptionsValidTest)
{
    req.target( (text_gen_arg.location) );
    req.method( http::verb::options );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::no_content);
}