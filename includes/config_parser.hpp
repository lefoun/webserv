#pragma once

#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <stack>
#include <dirent.h>
#include "helper_functions.hpp"
#include "class_Server.hpp"
#include "webserver.hpp"

enum DIRECTIVES {
	UNKNOWN_DIRECTIVE = -1,
	SERVER,
	LISTEN,
	SERVER_NAME,
	ROOT,
	INDEX,
	AUTO_INDEX,
	LOCATION,
	ERROR_DIRECTIVE,
	REDIRECTION,
	ALLOW,
	CLIENT_MAX_BODY_SIZE,
	DIRECTIVES_NB
};

const std::vector<std::string> init_directives();
void	init_host_ip_lookup(std::map<std::string, std::string>& host_ip_lookup);

template<typename T>
bool is_duplicate (std::vector<T> tab1, std::vector<T> tab2)
{
	if (tab1.size() == 0 && tab1.size() == 0)
		return true;

	typename std::vector<T>::iterator it1 = tab1.begin();
	while (it1 != tab1.end())
	{
		typename std::vector<T>::iterator it2 = tab2.begin();
		while (it2 != tab2.end())
		{
			if (*it1 == *it2)
				return true;
			++it2;
		}
		++it1;
	}
	return false;
}
void	check_errors(std::vector<Server>& servers);
void	enriche_configuration(std::vector<Server>& servers,
									std::map<std::string, std::string>&
									host_ip_lookup);
bool	parse_config_file(const std::string& file_name,
							std::vector<Server>& servers,
							std::map<std::string, std::string>& host_ip_lookup);

int	find_directive(const std::vector<std::string>& directives,
					const std::string& token);
void	check_valid_token(std::istream_iterator<std::string>& token);
void	set_port(const std::string& port_str, Server& server);
void	set_ip_port_pair(const std::string& host, const std::string& port,
							 Server& server,
							 const std::map<std::string, std::string>&
							 host_ip_lookup);
void	set_ip(const std::string& host, Server& server,
					const std::map<std::string, std::string>& host_ip_lookup);
void	set_allowed_method(const std::string& method,
								const std::string& context, Server& server);
template <typename T>
void	set_default_methods(T& block)
{
	block.get_allowed_methods().push_back("GET");
	// block.get_allowed_methods().push_back("POST");
	// block.get_allowed_methods().push_back("DELETE");
}
void	get_server(std::istream_iterator<std::string>& token,
						Server& server, std::stack<std::string>& context,
						const std::vector<std::string>& directives_vec,
						const std::map<std::string, std::string>& host_ip_lookup);
void	set_implicit_ip_port_pairs(std::vector<Server>& servers);


void	handle_error_directive(std::istream_iterator<std::string>& token,
								const std::stack<std::string>& context,
								Server& server);
void	handle_listen(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context,
						Server& server,
						const std::map<std::string, std::string>& host_ip_lookup);
void	handle_server_name(std::istream_iterator<std::string>& token,
							const std::stack<std::string>& context,
							Server& server);
void	handle_root(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context, Server& server);
void	handle_index(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context, Server& server);
void	handle_auto_index(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context, Server& server);
void	handle_location(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server);
void		handle_redirection(std::istream_iterator<std::string>& token,
								const std::stack<std::string>& context,
								Server& server);
void	handle_allow(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server);
void	handle_body_size_limit(std::istream_iterator<std::string>& token,
								const std::stack<std::string>& context,
								Server& server);



