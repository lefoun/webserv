#pragma once

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sstream>
#include "colors.hpp"

typedef struct request
{
	std::string	content_type;
	std::string	content_length;
	std::string	cookie;
	std::string	user_agent;
	std::string	path_info;
	std::string	query_string;
	std::string	remote_addr;
	std::string	remote_host;
	std::string	method;
	std::string	script_path;
	std::string	script_name;
	std::string	target;
	std::string	host;
	std::string	connection;
	std::string	body;
	in_addr_t	ip;
	uint16_t	port;

} request_t;

request_t*	get_parsed_request(const std::string& header);
