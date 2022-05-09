#include "gtest/gtest.h"
#include "config_parser.h"

#include <map>

class NginxConfigParserTest:public::testing::Test //Test fixture class, to initialize needed variable
{
  protected:
  NginxConfigParser parser;
  NginxConfig out_config;
  int port = -1;
  bool status;
  std::map<std::string, config_arg> addrmap;
};

TEST_F(NginxConfigParserTest, SimpleConfig){
  status = parser.Parse("config_files/example_config", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, Comments){
  status = parser.Parse("config_files/config_t_comment", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, CommentsInline){
  status = parser.Parse("config_files/config_t_comment_inline", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, SimpleDirective){
  status = parser.Parse("config_files/config_t_simple_directive", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, SimpleDirectiveFalse){
  status = parser.Parse("config_files/config_f_simple_directive", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, EmptyBracket){
  status = parser.Parse("config_files/config_t_emptybracket", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, SubDir){
  status = parser.Parse("config_files/config_t_subdir", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, SubDirMulti){
  status = parser.Parse("config_files/config_t_subdir_multi", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, WrongBracket){
  status = parser.Parse("config_files/config_f_WrongBracket", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, TrueSingleEscape){
  status = parser.Parse("config_files/config_t_single_escape", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==-1);  
}

TEST_F(NginxConfigParserTest, TrueDoubleEscape){
  status = parser.Parse("config_files/config_t_double_escape", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, FalseSingleEscape){
  status = parser.Parse("config_files/config_f_single_escape", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, FalseDoubleEscape){
  status = parser.Parse("config_files/config_f_double_escape", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, Complex){
  status = parser.Parse("config_files/config_t_complex", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, onlyPort){
  status = parser.Parse("config_files/config_t_port", &out_config, &port, &addrmap);
  EXPECT_TRUE(status);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, outRangePort){
  status = parser.Parse("config_files/config_f_port_out_range", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, largeIntPort){
  status = parser.Parse("config_files/config_f_port_large_int", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, stringPort){
  status = parser.Parse("config_files/config_f_port_string", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, tabNewline){
  status = parser.Parse("config_files/config_t_tab_newline", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}


TEST_F(NginxConfigParserTest, NoSemicolonConfig) {
  status = parser.Parse("config_files/config_f_no_semicolon", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, UnmatchBracketConfig) {
  status = parser.Parse("config_files/config_f_unmatch_bracket", &out_config, &port, &addrmap);
  EXPECT_FALSE(status);
}

TEST_F(NginxConfigParserTest, ToStringTest) {
  status = parser.Parse("config_files/config_t_subdir_multi", &out_config, &port, &addrmap);
  ASSERT_TRUE(status);
  const std::string expect_result = "server {\n  location / {\n    root /data/www;\n  }\n  "  
                                    "location /images/ {\n    root /data;\n    listen 80;\n  }\n}\n";
  std::string config_str = out_config.ToString();
  EXPECT_EQ(expect_result, config_str);
}

TEST_F(NginxConfigParserTest, FileNotExist) {
  status = parser.Parse("not_exist", &out_config, &port, &addrmap);
  ASSERT_FALSE(status);
}