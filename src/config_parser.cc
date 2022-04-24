// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <stdexcept> 
#include <map>
#include "config_parser.h"
#include "log.h"

int safeportSTOI(std::string stringnumber) {
	int result = -1;
	try
	{
		result = stoi(stringnumber);
	}
	catch(const std::invalid_argument& ia)
	{
		ERROR << "Config Parser: Port number not valid: not convertable" << std::endl;
	}
	catch(const std::out_of_range& outrange)
	{
		ERROR << "Config Parser: Port number not valid: number of of int range" << std::endl;
	}
	if(result < 0 || result > 65535)
	{
		ERROR << "Config Parser: Port number not valid: shoud be between 0 - 65535" << std::endl;
		result = -1;
	}
	return result;
}

std::string NginxConfig::ToString(int depth) {
	std::string serialized_config;
	for (const auto& statement : statements_) {
		serialized_config.append(statement->ToString(depth));
	}
	return serialized_config;
}

std::string NginxConfigStatement::ToString(int depth) {
	std::string serialized_statement;
	for (int i = 0; i < depth; ++i) {
		serialized_statement.append("  ");
	}
	for (unsigned int i = 0; i < tokens_.size(); ++i) {
		if (i != 0) {
			serialized_statement.append(" ");
		}
		serialized_statement.append(tokens_[i]);
	}
	if (child_block_.get() != nullptr) {
		serialized_statement.append(" {\n");
		serialized_statement.append(child_block_->ToString(depth + 1));
		for (int i = 0; i < depth; ++i) {
			serialized_statement.append("  ");
		}
		serialized_statement.append("}");
	} else {
		serialized_statement.append(";");
	}
	serialized_statement.append("\n");
	return serialized_statement;
}

const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
	switch (type) {
		case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
		case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
		case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
		case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
		case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
		case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
		case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
		case TOKEN_TYPE_ERROR:         return "TOKEN_TYPE_ERROR";
		default:                       return "Unknown token type";
	}
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input, std::string* value) {
	TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
	while (input->good()) {
		const char c = input->get();
		if (!input->good()) {
			break;
		}
		switch (state) {
			case TOKEN_STATE_INITIAL_WHITESPACE:
				switch (c) {
				case '{':
					*value = c;
					return TOKEN_TYPE_START_BLOCK;
				case '}':
					*value = c;
					return TOKEN_TYPE_END_BLOCK;
				case '#':
					*value = c;
					state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
					continue;
				case '"':
					*value = c;
					state = TOKEN_STATE_DOUBLE_QUOTE;
					continue;
				case '\'':
					*value = c;
					state = TOKEN_STATE_SINGLE_QUOTE;
					continue;
				case ';':
					*value = c;
					return TOKEN_TYPE_STATEMENT_END;
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					continue;
				default:
					*value += c;
					state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
					continue;
				}
			case TOKEN_STATE_SINGLE_QUOTE:
				if (c =='\\'){
					if(input->good()){
						char c2 = input->get();
						*value += c2;
						continue;
					}
					else{
						return TOKEN_TYPE_ERROR;
					}
				}
				else{
					*value += c;
					if (c == '\'') {
						if(input->good()){
							char c2 = input->get();
							if (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\t' ||
								c2 == ';' || c2 == '{' || c2 == '}') {
								input->unget();
								return TOKEN_TYPE_NORMAL;
							}
							else{
								return TOKEN_TYPE_ERROR;
							}    
						} 
						continue;
					}
					continue;
				}
			case TOKEN_STATE_DOUBLE_QUOTE:
				if (c =='\\'){
					if(input->good()){
						char c2 = input->get();
						*value += c2;
						continue;
					}
					else{
						return TOKEN_TYPE_ERROR;
					}
				}
				else{
					*value += c;
					if (c == '"') {
						if(input->good()){
							char c2 = input->get();
							if (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '\t' ||
								c2 == ';' || c2 == '{' || c2 == '}') {
								input->unget();
								return TOKEN_TYPE_NORMAL;
							}
							else{
								return TOKEN_TYPE_ERROR;
							}    
						} 
						continue;
					}
					continue;
				}
			case TOKEN_STATE_TOKEN_TYPE_COMMENT:
				if (c == '\n' || c == '\r') {
					return TOKEN_TYPE_COMMENT;
				}
				*value += c;
				continue;
			case TOKEN_STATE_TOKEN_TYPE_NORMAL:
				if (c == ' ' || c == '\t' || c == '\n' || c == '\t' ||
					c == ';' || c == '{' || c == '}') {
					input->unget();
					return TOKEN_TYPE_NORMAL;
				}
				*value += c;
				continue;
		}
	}

	// If we get here, we reached the end of the file.
	if (state == TOKEN_STATE_SINGLE_QUOTE ||
		state == TOKEN_STATE_DOUBLE_QUOTE) {
		return TOKEN_TYPE_ERROR;
	}

	return TOKEN_TYPE_EOF;
}

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config, int* port, std::map<std::string, std::string>* addrmap) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  int remaining_bracket = 0;
  *port = -1;
//   *basepath = "";
  while (true) {
	std::string token;
	token_type = ParseToken(config_file, &token);
	if (token_type == TOKEN_TYPE_ERROR) {
		break;
	}

	if (token_type == TOKEN_TYPE_COMMENT) {
		// Skip comments.
		continue;
	}
	if (token_type == TOKEN_TYPE_START) {
		// Error.
		break;
	} else if (token_type == TOKEN_TYPE_NORMAL) {
	  	if (last_token_type == TOKEN_TYPE_START ||
			last_token_type == TOKEN_TYPE_STATEMENT_END ||
			last_token_type == TOKEN_TYPE_START_BLOCK ||
			last_token_type == TOKEN_TYPE_END_BLOCK ||
			last_token_type == TOKEN_TYPE_NORMAL) {
			if (last_token_type != TOKEN_TYPE_NORMAL) {
				config_stack.top()->statements_.emplace_back(new NginxConfigStatement);
			}
			if(!config_stack.top()->statements_.back().get()->tokens_.empty()){
				std::string lastoken = config_stack.top()->statements_.back().get()->tokens_.back();
				if((last_token_type == TOKEN_TYPE_NORMAL) && (lastoken.compare("listen") == 0)){
					int currport = safeportSTOI(token);
					if(currport != -1){
						*port = currport;
					} else {
						// Invalid port number, error
						break;
					}
				}
				else if((last_token_type == TOKEN_TYPE_NORMAL) && (lastoken.compare("root") == 0)){
					std::string location = "";
					NginxConfig* currentlayer = config_stack.top();
					config_stack.pop();
					if(config_stack.empty()){
						INFO << "Config: root is not inside the child bracket of location, using default location: /static\n";
						location = "/static";
					}
					else{
						std::vector<std::string> parent_token = config_stack.top()->statements_.back().get()->tokens_;
						if(parent_token.rbegin()[1].compare("location") == 0){
							location = parent_token.rbegin()[0];
						}
						else{
							INFO << "Config: root is not inside the child bracket of location, using default location: /static\n";
							location = "/static";	
						}
					}
					if(addrmap->count(location) > 0){
						WARNING << "Location: "<< location << " has already be mapped to a path, ignored\n"; 
					}
					else{
						addrmap->insert(std::pair<std::string,std::string>(location,token));
						INFO << "Map location: " << location  << " with root: " << token << "\n";
					}
					config_stack.push(currentlayer);
				}
			}
			config_stack.top()->statements_.back().get()->tokens_.push_back(token);

	  	} else {
			// Error.
			break;
		}
	} else if (token_type == TOKEN_TYPE_STATEMENT_END) {
		if (last_token_type != TOKEN_TYPE_NORMAL) {
			// Error.
			break;
		}
	} else if (token_type == TOKEN_TYPE_START_BLOCK) {
		remaining_bracket += 1;
		NginxConfig* const new_config = new NginxConfig;
		config_stack.top()->statements_.back().get()->child_block_.reset(new_config);
		config_stack.push(new_config);
	} else if (token_type == TOKEN_TYPE_END_BLOCK) {
		remaining_bracket -= 1;
		if (last_token_type != TOKEN_TYPE_STATEMENT_END 
			&& last_token_type !=TOKEN_TYPE_START_BLOCK 
			&& last_token_type != TOKEN_TYPE_END_BLOCK) {
			// Error.
			break;
		}
	  	config_stack.pop();
	} else if (token_type == TOKEN_TYPE_EOF) {
	  	if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
		  	last_token_type != TOKEN_TYPE_END_BLOCK) {
			// Error.
			break;
		}
		else if (remaining_bracket != 0) {//bracket mismatch
			break;
		}
		return true;
	} else {
		// Error. Unknown token.
		break;
	}
	last_token_type = token_type;
  }

  ERROR << "Config Parser: Bad transition from " << TokenTypeAsString(last_token_type) << " to " <<  TokenTypeAsString(token_type) << "\n";
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config, int* port, std::map<std::string, std::string>* addrmap) {
	std::ifstream config_file;
	config_file.open(file_name);
	if (!config_file.good()) {
		ERROR << "Config Parser: Failed to open config file " << file_name << "\n";
		return false;
	}
	const bool return_value = Parse(dynamic_cast<std::istream*>(&config_file), config, port, addrmap);
	if(addrmap->size() == 0){
		WARNING << "Static address binding is empty! Please check the config file!\n";
	}
	config_file.close();
	return return_value;
}
