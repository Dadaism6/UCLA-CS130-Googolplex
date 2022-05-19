#include "gtest/gtest.h"
#include "config_arg.h"
#include "request_handler_crud.h"

#include <fstream>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

class CrudHandlerTest :public::testing::Test
{
    public:
        CrudHandlerTest() {
            crud_arg.root = "../crud_data";
            crud_arg.location = "/api";
            req_handler_crud = new request_handler_crud(crud_arg.location, crud_arg.root, file_to_id);
            if(!boost::filesystem::exists(crud_arg.root))
            {
                boost::filesystem::path rootFolder = crud_arg.root;
                boost::filesystem::create_directory(rootFolder);
            }
        }
    
        ~CrudHandlerTest() {
            delete req_handler_crud;
        }
    
        std::string not_found = "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
        http::request<http::string_body> req;
        http::response<http::string_body> rep;
        enum {content_length_field = 0, content_type_field = 1};
    
        std::map<std::string, std::vector<int>> file_to_id;

        config_arg crud_arg;
        request_handler* req_handler_crud;
};


TEST_F(CrudHandlerTest, CrudHandlerPOSTTest)
{
    req.target( "/api/Shoes");
    req.method( http::verb::post );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::created);
    EXPECT_EQ(std::string(rep.body().data()), "Created entry at {\"id\":1}\n");
}

TEST_F(CrudHandlerTest, CrudHandlerSequentialPOSTTest)
{
    req.target( "/api/Shoes");
    req.method( http::verb::post );
    req_handler_crud -> handle_request(req, rep);
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/2" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1, 2};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::created);
    EXPECT_EQ(std::string(rep.body().data()), "Created entry at {\"id\":2}\n");
}

TEST_F(CrudHandlerTest, CrudHandlerPOSTDirCreationFailTest)
{
    req.target( "/api/Shoes");
    req.method( http::verb::post );
    req_handler_crud -> handle_request(req, rep);
    req.target( "/api/Shoes/1");
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1/1" ), false);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::unprocessable_entity);
    EXPECT_EQ(std::string(rep.body().data()), "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>");
}

TEST_F(CrudHandlerTest, CrudHandlerPOSTSmallestIDTest)
{
    file_to_id["Shoes"] = {2};
    boost::filesystem::create_directory("../crud_data/Shoes");
    std::ofstream file("../crud_data/Shoes/2");
    file << "test";
    file.close();
    request_handler* req_handler_crud_1 = new request_handler_crud(crud_arg.location, crud_arg.root, file_to_id);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), false);
    req.target( "/api/Shoes");
    req.method( http::verb::post );
    req_handler_crud_1 -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/2" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1, 2};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::created);
    EXPECT_EQ(std::string(rep.body().data()), "Created entry at {\"id\":1}\n");
    delete req_handler_crud_1;
}

TEST_F(CrudHandlerTest, CrudHandlerPUTInvalidFileTest)
{
    req.target( "/api/Shoes/a");
    req.method( http::verb::put );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
}

TEST_F(CrudHandlerTest, CrudHandlerPUTNewEntityTest)
{
    req.target( "/api/Shoes/2");
    req.method( http::verb::put );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/2" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {2};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::created);
    EXPECT_EQ(std::string(rep.body().data()), "Created entry at {\"id\":2}\n");
}

TEST_F(CrudHandlerTest, CrudHandlerPUTDirCreationFailTest)
{
    req.target( "/api/Shoes/1");
    req.method( http::verb::put );
    req_handler_crud -> handle_request(req, rep);
    req.target( "/api/Shoes/1/1");
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1/1" ), false);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::unprocessable_entity);
    EXPECT_EQ(std::string(rep.body().data()), "<html><head><title>Unprocessable Entity</title></head><body><h1>422 Unprocessable Entity</h1></body></html>");
}

TEST_F(CrudHandlerTest, CrudHandlerPUTNewFileExistingEntityTest)
{
    req.target( "/api/Shoes/1");
    req.method( http::verb::put );
    req_handler_crud -> handle_request(req, rep);
    req.target( "/api/Shoes/2");
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/2" ), true);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1, 2};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::created);
    EXPECT_EQ(std::string(rep.body().data()), "Created entry at {\"id\":2}\n");
}

TEST_F(CrudHandlerTest, CrudHandlerPUTExistingFileTest)
{
    req.target( "/api/Shoes/1");
    req.method( http::verb::put );
    req_handler_crud -> handle_request(req, rep);
    req.target( "/api/Shoes/1");
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "Updated entry at {\"id\":1}\n");
}

TEST_F(CrudHandlerTest, CrudHandlerGETListTest)
{
    req.target( "/api/Shoes" );
    req.method( http::verb::post );
    req_handler_crud -> handle_request(req, rep);
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/2" ), true);
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1, 2};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    req.target( "/api/Shoes" );
    req.method( http::verb::get );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "[1,2]\n");  
    boost::filesystem::remove_all("../crud_data/Shoes");
}

TEST_F(CrudHandlerTest, CrudHandlerGETReadTest)
{
    boost::filesystem::create_directory("../crud_data/Shoes");
    std::ofstream file("../crud_data/Shoes/1");
    file << "test";
    file.close();
    file_to_id["Shoes"] = {1};
    req.target( "/api/Shoes/1" );
    req.method( http::verb::get );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::ok);
    EXPECT_EQ(std::string(rep.body().data()), "test");
    boost::filesystem::remove_all("../crud_data/Shoes");
}

TEST_F(CrudHandlerTest, CrudHandlerGETNotFoundTest)
{
    req.target( "/api/Shoes" );
    req.method( http::verb::get );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(rep.result(), http::status::not_found);
    EXPECT_EQ(std::string(rep.body().data()), not_found);
}

TEST_F(CrudHandlerTest, CrudHandlerDELETETest)
{
    boost::filesystem::create_directory("../crud_data/Shoes");
    std::ofstream file("../crud_data/Shoes/1");
    file << "test";
    file.close();
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    file_to_id["Shoes"] = {1};
    req.target( "/api/Shoes/1" );
    req.method( http::verb::delete_ );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), false);
    std::vector<int> ids = {};
    EXPECT_EQ(file_to_id["Shoes"], ids);
    EXPECT_EQ(std::string(rep.body().data()), "Successfully deleted file at /Shoes.\n");
    EXPECT_EQ(rep.result(), http::status::ok);
    boost::filesystem::remove_all("../crud_data/Shoes");
}

TEST_F(CrudHandlerTest, CrudHandlerDELETEDirectoryTest)
{
    boost::filesystem::create_directory("../crud_data/Shoes");
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes" ), true);
    req.target( "/api/Shoes" );
    req.method( http::verb::delete_ );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(std::string(rep.body().data()), "Invalid format; expected a file ID.\n");
    EXPECT_EQ(rep.result(), http::status::bad_request);
    boost::filesystem::remove_all("../crud_data/Shoes");
}

TEST_F(CrudHandlerTest, CrudHandlerDELETENotExistingTest)
{
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), false);
    req.target( "/api/Shoes/1" );
    req.method( http::verb::delete_ );
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(std::string(rep.body().data()), not_found);
    EXPECT_EQ(rep.result(), http::status::not_found);
}
