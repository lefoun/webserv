#pragma once

#include <string>
#include <sys/types.h>
#include <sstream>

typedef struct request
{
	std::string	method;
	std::string	args;
	std::string	target;
	std::string	host;
	std::string	connection;
	in_addr_t	ip;
	uint16_t	port;

} request_t;

request_t*	get_parsed_request(const char buffer[]);
