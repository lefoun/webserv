#pragma once

#include <iostream>
#include <sys/types.h>
#include "request_parsing.hpp"

typedef struct
{
	uint8_t			response_state;
	uint16_t		return_code;
	std::string		return_message;
	bool			is_auto_index;
	bool			is_chunked;
	std::string		date;
	std::string		content_type;
	std::string		location;
	std::string		body;
	std::string		file_path;
	std::string		server;
} response_t;


std::string	generate_cookie(const size_t size = 32);
void	send_chunked_response(response_t* response, std::string& response_str,
								const int& socket_fd);
void	construct_header(response_t* response, request_t* request,
							std::string& header);
std::string	get_content_type(const std::string& file_extension);
void	send_response(request_t* request, const int& socket_fd,
						response_t* response);
void	initialize_html_return_code_page(t_return_codes *return_codes);
void		set_response(Server & server, request_t* request,
									response_t* response);


void	set_cgi_env_variables(const request_t* request);

void	get_cgi_response(const request_t* request, response_t* response,
							std::string& response_str, const int& socket_fd);

void	get_cgi_php_response(request_t* request, response_t* response,
							std::string& response_str, const int& socket_fd);

std::string get_body_auto_index(std::string full_path, std::string dir_path);
