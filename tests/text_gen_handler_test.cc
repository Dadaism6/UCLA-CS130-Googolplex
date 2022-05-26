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
            text_gen_arg.root = "../crud_data";
            text_gen_arg.api_key = "c97c056c-bb68-4ee5-82b6-97bd2e47c1b6";
            req_handler_text_gen = new request_handler_text_gen(text_gen_arg.location, text_gen_arg.root, text_gen_arg.api_key);
        }
    
        ~TextGenHandlerTest() {
            delete req_handler_text_gen;
        }
    
        std::string not_found = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
        http::request<http::string_body> req;
        http::response<http::string_body> rep;
        enum {content_length_field = 0, content_type_field = 1};

        config_arg text_gen_arg;
        request_handler* req_handler_text_gen;
};

TEST_F(TextGenHandlerTest, TextGenHandlerInvalidMethodTest)
{
    req.target( "/text_gen" );
    req.method( http::verb::trace );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}

TEST_F(TextGenHandlerTest, TextGenHandlerPostInvalidTest)
{
    req.target( "/text_gen" );
    req.method( http::verb::post );
    req_handler_text_gen -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::bad_request);
}