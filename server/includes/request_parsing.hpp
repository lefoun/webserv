#pragma once

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sstream>
#include "helper_functions.hpp"
#include "colors.hpp"

#define NOT_STARTED 0
#define INCOMPLETE 1
#define COMPLETE 2

#define UNKNOWN 0
#define CHUNKED 1
#define UNCHUNKED 2

enum request_keys_e
{
	GET,
	POST,
	DELETE,
	CONTENT_TYPE,
	CONTENT_LENGTH,
	COOKIE,
	USER_AGENT,
	CONNECTION,
	HOST,
	PROTOCOL,
	TRACKING_COOKIE,
	BOUNDARY,
	TRANSFER_ENCODING,
	REQUEST_KEYS_SIZE
};
#include <netinet/in.h>

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
	std::string	boundary;
	std::string	transfer_encoding;
	in_addr_t	ip;
	uint16_t	port;
	int			body_parsing_state;
} request_t;


void		parse_request_header(std::string& header, request_t* request);
void		parse_request_body(std::string& client_req, request_t* request);
request_t*	get_parsed_request(const std::string& header);
