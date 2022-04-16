#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest:public::testing::Test //Test fixture class, to initialize needed variable
{
  protected:
  NginxConfigParser parser;
  NginxConfig out_config;
  int port = -1;
};

TEST_F(NginxConfigParserTest, SimpleConfig){
  bool success = parser.Parse("example_config", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, Comments){
  bool success = parser.Parse("config_t_comment", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, CommentsInline){
  bool success = parser.Parse("config_t_comment_inline", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, SimpleDirective){
  bool success = parser.Parse("config_t_simple_directive", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, SimpleDirectiveFalse){
  bool success = parser.Parse("config_f_simple_directive", &out_config, &port);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, EmptyBracket){
  bool success = parser.Parse("config_t_emptybracket", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, SubDir){
  bool success = parser.Parse("config_t_subdir", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, SubDirMulti){
  bool success = parser.Parse("config_t_subdir_multi", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, WrongBracket){
  bool success = parser.Parse("config_f_WrongBracket", &out_config, &port);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, TrueSingleEscape){
  bool success = parser.Parse("config_t_single_escape", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==-1);  
}

TEST_F(NginxConfigParserTest, TrueDoubleEscape){
  bool success = parser.Parse("config_t_double_escape", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==-1);
}

TEST_F(NginxConfigParserTest, FalseSingleEscape){
  bool success = parser.Parse("config_f_single_escape", &out_config, &port);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, FalseDoubleEscape){
  bool success = parser.Parse("config_f_double_escape", &out_config, &port);
  EXPECT_FALSE(success);
}

TEST_F(NginxConfigParserTest, Complex){
  bool success = parser.Parse("config_t_complex", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, onlyPort){
  bool success = parser.Parse("config_t_port", &out_config, &port);
  EXPECT_TRUE(success);
  EXPECT_TRUE(port==80);
}

TEST_F(NginxConfigParserTest, outRangePort){
  bool success = parser.Parse("config_f_port_out_range", &out_config, &port);
  EXPECT_FALSE(success);
}