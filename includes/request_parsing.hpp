#pragma once

#include <string>
#include <iostream>
#include <limits>
#include <sys/types.h>
#include <sstream>
#include "colors.hpp"
#include <netinet/in.h>
#include <map>
#include "class_Server.hpp"

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
	CONNECTION,
	HOST,
	PROTOCOL,
	SESSION_COOKIE,
	TRACKING_COOKIE,
	BOUNDARY,
	TRANSFER_ENCODING,
	REQUEST_KEYS_SIZE
};

typedef struct request
{
	std::string	content_type;
	size_t		content_length;
	bool		is_content_length_set;
	std::string	session_cookie;
	std::string	permanent_cookie;
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
	std::string	referer;
	in_addr_t	ip;
	uint16_t	port;
	int			body_parsing_state;
} request_t;


void		parse_request_header(std::string& header, request_t* request,
									const std::map<std::string, std::string>&
									host_ip_lookup, const char* lookup[REQUEST_KEYS_SIZE]);

void		parse_request_body(std::string& client_req, request_t* request);
request_t*	get_parsed_request(const std::string& header);

void clear_request(request_t & request);

void	init_request_parsing_lookup_tab(const char *lookup[REQUEST_KEYS_SIZE]);

bool	is_complete_request(std::string& request, request_t *rqst,
							const std::map<std::string, std::string>&
							host_ip_lookup,
							const char *req_parsing_lookup[REQUEST_KEYS_SIZE]);

Server*	get_server_associated_with_request(std::vector<Server>& servers,
											const request_t *request);

