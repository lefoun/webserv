#pragma once

/* C libraries to enable socket libraries */
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <ctime>

/* Socket | Inet libraries to enable communication */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

/* C++ Libraries to enable I/O */
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <ios>

/* C++ Libraries to enable container creation and algorithm use */
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <utility>

#include "helper_functions.hpp"
#include "class_Server.hpp"
#include "request_parsing.hpp"
#include "colors.hpp"
#include "response.hpp"
#include "class_Socket.hpp"
#include "sockets.hpp"

/* for the buffer that reads the clients' message */
// #define BUFFER_SIZE 30
// #define BUFFER_SIZE 127670
// #define BUFFER_SIZE 4096
#define BUFFER_SIZE 65536 
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF "\r\n"

class Socket;
class SockComm;

bool	parse_config_file(const std::string& file_name,
							std::vector<Server>& servers,
							std::map<std::string, std::string>& host_ip_lookup);

void	initialize_html_return_code_page(t_return_codes *return_codes);

void		set_response(Server & server, request_t* request,
									response_t* response);
Location	*choose_location(Server & server, request_t* request);
void		change_default_html_return_code(std::string path,
									std::string *return_code);
void	set_location_options(Server & server, request_t* request,
									Location & location, response_t* response);
void	choose_return_code_for_requested_ressource(Server& server,
									request_t* request, response_t* response);

template <typename T>
void	set_default_return_code(T & datas)
{
	std::vector<uint16_str_pair>::iterator it;
	std::vector<uint16_str_pair>::iterator it_end;
	std::string root_path;
	t_return_codes *return_codes;

	it = datas.get_error_pages().begin();
	it_end = datas.get_error_pages().end();
	return_codes = &datas.return_codes;
	initialize_html_return_code_page(&(*return_codes));
	root_path = datas.get_root_path();
	if (--(*root_path.end()) != '/')
		root_path += '/';
	for (; it != it_end; ++it)
	{
		switch (it->first)
		{
			case 400:
				change_default_html_return_code(root_path + it->second, &return_codes->err_400);
				break;
			case 401:
				change_default_html_return_code(root_path + it->second, &return_codes->err_401);
				break;
			case 403:
				change_default_html_return_code(root_path + it->second, &return_codes->err_403);
				break;
			case 404:
				change_default_html_return_code(root_path + it->second, &return_codes->err_404);
				break;
			case 405:
				change_default_html_return_code(root_path + it->second, &return_codes->err_405);
				break;
			case 406:
				change_default_html_return_code(root_path + it->second, &return_codes->err_406);
				break;
			case 408:
				change_default_html_return_code(root_path + it->second, &return_codes->err_408);
				break;
			case 413:
				change_default_html_return_code(root_path + it->second, &return_codes->err_413);
				break;
			case 414:
				change_default_html_return_code(root_path + it->second, &return_codes->err_414);
				break;
			case 429:
				change_default_html_return_code(root_path + it->second, &return_codes->err_429);
				break;
			case 431:
				change_default_html_return_code(root_path + it->second, &return_codes->err_431);
				break;
			case 500:
				change_default_html_return_code(root_path + it->second, &return_codes->err_500);
				break;
			case 501:
				change_default_html_return_code(root_path + it->second, &return_codes->err_501);
				break;
			case 502:
				change_default_html_return_code(root_path + it->second, &return_codes->err_502);
				break;
			case 503:
				change_default_html_return_code(root_path + it->second, &return_codes->err_503);
				break;
			case 504:
				change_default_html_return_code(root_path + it->second, &return_codes->err_504);
				break;
			case 505:
				change_default_html_return_code(root_path + it->second, &return_codes->err_505);
				break;
			case 511:
				change_default_html_return_code(root_path + it->second, &return_codes->err_511);
				break;
			default:
				break;
		}
	}
}
