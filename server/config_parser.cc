#include "config_parser.hpp"

const std::vector<std::string> init_directives()
{
	std::vector<std::string> directives(DIRECTIVES_NB);

	directives[SERVER] = "server";
	directives[LISTEN] = "listen";
	directives[SERVER_NAME] = "server_name";
	directives[ROOT] = "root";
	directives[INDEX] = "index";
	directives[AUTO_INDEX] = "auto_index";
	directives[LOCATION] = "location";
	directives[ERROR_DIRECTIVE] = "error_page";
	directives[REDIRECTION] = "redirect";
	directives[ALLOW] = "allow";
	directives[CLIENT_MAX_BODY_SIZE] = "client_max_body_size";
	return directives;
}

static int	find_directive(const std::vector<std::string>& directives,
					const std::string& token)
{
	for (ssize_t i = 0; i < DIRECTIVES_NB; ++i)
		if (directives[i] == token)
			return i;
	return UNKNOWN_DIRECTIVE;
}

static void	check_valid_token(std::istream_iterator<std::string>& token)
{
	std::istream_iterator<std::string> end_of_file;
	if (++token == end_of_file)
		throw std::invalid_argument("Unexpected end of file");
	if (*(token->begin()) == ';')
		throw std::invalid_argument("Unexcpected token ';'");
}

static void	set_port(const std::string& port_str, Server& server)
{
	if (port_str.size() > 5)
	{
		std::cout << port_str << std::endl;
		throw std::invalid_argument("Port number out of range");
	}
	if (!in_range(1, 65535, atoi(port_str.c_str())))
		throw std::invalid_argument("Invalid Port number");
	(server.get_listening_ports()).push_back(atoi(port_str.c_str()));
}

static void	set_ip_port_pair(const std::string& host, const std::string& port,
							 Server& server,
							 const std::map<std::string, std::string>&
							 host_ip_lookup)
{
	std::string	ip_host = host;

	if (!is_ip_address(host))
	{
		if (host_ip_lookup.find(host) == host_ip_lookup.end())
			throw std::invalid_argument("Can't find host");
		ip_host = (host_ip_lookup.find(host))->second;
	}
	if (port.size() > 5 || !in_range(1, 65535, atoi(port.c_str())))
		throw std::invalid_argument("Invalid port number");
	server.get_ip_port_pairs().push_back(std::make_pair(
		ip_host, atoi(port.c_str())));
}

static void	set_ip(const std::string& host, Server& server,
					const std::map<std::string, std::string>& host_ip_lookup)
{
	if (is_ip_address(host))
	{
		server.get_listening_ips().push_back(host);
		return ;
	}

	if (host_ip_lookup.find(host) == host_ip_lookup.end())
		throw std::invalid_argument("Unable to find host");
	const std::string host_ip = (host_ip_lookup.find(host))->second;
	server.get_listening_ips().push_back(host_ip);
}

static void	set_allowed_method(const std::string& method,
								const std::string& context, Server& server)
{
	if (method != "GET" && method != "POST" && method != "DELETE")
		throw std::invalid_argument("Unkown method " + method);
	if (context == "server")
	{
		if (!is_in_vector(server.get_allowed_methods(), method))
			server.get_allowed_methods().push_back(method);
	}
	else if (context == "location")
	{
		if (!is_in_vector(server.get_locations().back().get_allowed_methods(),
			method))
			server.get_locations().back().get_allowed_methods().push_back(
				method);
	}
}

template <typename T>
static void	set_default_methods(T& block)
{
	block.get_allowed_methods().push_back("GET");
	// block.get_allowed_methods().push_back("POST");
	// block.get_allowed_methods().push_back("DELETE");
}

static void	handle_error_directive(std::istream_iterator<std::string>& token,
								const std::stack<std::string>& context,
								Server& server)
{
	check_valid_token(token);
	std::istream_iterator<std::string> end_of_file;
	if (!is_number(*token)) 
		throw std::invalid_argument("Unexpected token" + *token);
	std::string error_nb = *token;
	std::string error_page = *++token;
	if (error_page[error_page.size() - 1] != ';')
		throw std::invalid_argument("Expected token ';'");
	error_page = token->substr(0, token->size() - 1);
	if (error_nb.size() > 3 || !is_number(error_nb)
		|| !in_range(400, 550, atoi(error_nb.c_str())))
		throw std::invalid_argument("Invalid error number");

	uint16_t error = atoi(error_nb.c_str());
	if (context.top() == "server")
		server.get_error_pages().push_back(std::make_pair(error, error_page));
	else if (context.top() == "location")
		server.get_locations().back().get_error_pages().push_back(
			std::make_pair(error, error_page));
	std::cout << "This is error page " << error_page << std::endl;
	if (token == end_of_file)
		throw std::invalid_argument("unexpected end of file");
	++token;
}

static void	handle_listen(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context,
						Server& server,
						const std::map<std::string, std::string>& host_ip_lookup)
{
	check_valid_token(token);
	if (context.top() != "server")
		throw std::invalid_argument("Unexpected token inside location block");
	if (*(--(*token).end()) != ';')	
		throw std::invalid_argument("Expected token ';'");
	std::string	trimmed_token = *token;
	/* Erase the trailing ';' */
	trimmed_token.erase((trimmed_token.size() - 1));
	/* Listen either to a Host, a Port number or a Host:Port pair */
	std::string::size_type	pos = trimmed_token.find(':');
	if (pos == std::string::npos) /* Means that it's either a Host or a Port */
	{
		std::cout << "This is either a port or a host "
					<< trimmed_token << std::endl;
		if (is_number(trimmed_token))
			set_port(trimmed_token, server);
		else
			set_ip(trimmed_token, server, host_ip_lookup);
	}
	else /* It's a host:port pair */
	{
		std::cout << "This is a ip:port pair " << trimmed_token << std::endl;
		std::string	host = trimmed_token.substr(0, pos);
		std::string	port = trimmed_token.substr(pos + 1, std::string::npos);
		set_ip_port_pair(host, port, server, host_ip_lookup);
	}
	++token;
}

static void	handle_server_name(std::istream_iterator<std::string>& token,
							const std::stack<std::string>& context,
							Server& server)
{
	check_valid_token(token);
	std::istream_iterator<std::string> end_of_file;
	if (context.top() != "server")
		throw std::invalid_argument(
			"unexpected token in " + context.top() + " block");
	while (*(--(*token).end()) != ';' && token != end_of_file)
		server.get_server_names().push_back(*token++);
	if (token == end_of_file)
		throw std::invalid_argument("unexpected end of file");
	std::cout << "this is sever_nme " << *token << std::endl;
	server.get_server_names().push_back(
									(*token).substr(0, (*token).size() - 1));
	++token;
}

static void	handle_root(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context, Server& server)
{
	check_valid_token(token);
	if (*(--(*token).end()) != ';')
		throw std::invalid_argument("Expected token ';'");
	std::string	trimmed_token = token->substr(0, token->size() - 1);
	if (trimmed_token == "\"\"")
		throw std::invalid_argument("Invalid root path \"\"");
	std::cout << "This is root " << trimmed_token << std::endl;
	if (context.top() == "server")
	{
		if (!server.get_root_path().empty())
			throw std::invalid_argument(
				"Multiple Root directives is not allowed");
		server.get_root_path() = trimmed_token;
	}
	else /*the context is a location block */
	{
		if (!server.get_locations().back().get_root_path().empty())
			throw std::invalid_argument(
				"Multiple Root directives is not allowed");
		server.get_locations().back().get_root_path() = trimmed_token;
	}
	++token;
}

static void	handle_index(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context, Server& server)
{
	check_valid_token(token);
	if (*(--(*token).end()) != ';')
		throw std::invalid_argument("Expected token ';'");
	std::string	trimmed_token = token->substr(0, token->size() - 1);
	if (trimmed_token == "\"\"")
		throw std::invalid_argument("Invalid index \"\"");
	std::cout << "This is index " << trimmed_token << std::endl;
	if (context.top() == "server")
	{
		if (!server.get_index_file().empty())
			throw std::invalid_argument(
				"Multiple Index directives is not allowed in server block");
		server.get_index_file() = trimmed_token;
	}
	else /*the context is a location block */
	{
		if (!server.get_locations().back().get_index_file().empty())
			throw std::invalid_argument(
				"Multiple Index directives is not allowed in location block");
		(server.get_locations().back()).get_index_file() = trimmed_token;
	}
	++token;
}

static void	handle_auto_index(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context, Server& server)
{
	check_valid_token(token);
	if (*(--(*token).end()) != ';')
		throw std::invalid_argument("Expected token ';'");
	std::string	trimmed_token = token->substr(0, token->size() - 1);
	std::cout << "Auto index is " << trimmed_token << std::endl;
	if (trimmed_token != "off" && trimmed_token != "on")
		throw std::invalid_argument("Bad argument for auto_index");
	if (context.top() == "server")
	{
		if (server.get_is_auto_index_set())
			throw std::invalid_argument(
				"Multiple auto_index directives in server block \
				is not allowed");
		server.get_is_auto_index_set() = true;
		if (trimmed_token == "on")
			server.get_auto_index() = true;
	}
	else if (context.top() == "location")
	{
		if (server.get_locations().back().get_is_auto_index_set())
			throw std::invalid_argument(
				"Multiple auto_index directives in location block \
				is not allowed");
		server.get_locations().back().get_is_auto_index_set() = true;
		if (trimmed_token == "on")
			server.get_locations().back().get_auto_index() = true;
	}
	++token;
}

static void	handle_location(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	std::cout << "Inside Location block\n";
	check_valid_token(token);
	if (*(token->begin()) == '{') /* in case location is empty */
		throw std::invalid_argument("Expected path near location block");
	server.get_locations().push_back(Location(*token));
	std::cout << "This is location " << *token  << "\n\n" << std::endl;
	++token;
	if (*(token->begin()) != '{')
		throw std::invalid_argument("Expected token '{' near location block");
	std::istream_iterator<std::string> end_of_file;
	if (++token == end_of_file)
		throw std::invalid_argument(
			"Unexpected end of file near location block"); 
	context.push("location");
}

static void    handle_redirection(std::istream_iterator<std::string>& token,
                            const std::stack<std::string>& context,
                            Server& server)
{
	std::cout << "This is toekn redirection " << *token << std::endl;
    check_valid_token(token);
    if (*(--(*token).end()) != ';')
        throw std::invalid_argument("Expected token ';'");
    const std::string new_url = token->substr(0, token->size() - 1);
    if (context.top() == "server")
    {
        if (!server.get_redirections().empty())
            throw std::invalid_argument("Found multipe redirections in block server");
        server.get_redirections() = new_url;
        std::cout << server.get_redirections() << std::endl; 
    }
    else if (context.top() == "location")
    {
        if (!server.get_locations().back().get_redirections().empty())
            throw std::invalid_argument("Found multipe redirections in block location");
        server.get_locations().back().get_redirections() = new_url;
        std::cout << server.get_locations().back().get_redirections() << std::endl; 
    }
    ++token;
}

static void	handle_allow(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	check_valid_token(token);
	std::istream_iterator<std::string>	end_of_file;
	std::cout << "This is allowd method " << *token << std::endl;
	while (*(--(token->end())) != ';' && token != end_of_file)
		set_allowed_method(*token++, context.top(), server);
	if (token == end_of_file)
		throw std::invalid_argument("Unexpected end of file");
	set_allowed_method(token->substr(0, token->size() - 1), context.top(),
						server);
	++token;
}

static void	handle_body_size_limit(std::istream_iterator<std::string>& token,
								const std::stack<std::string>& context,
								Server& server)
{
	check_valid_token(token);
	if (*(--(*token).end()) != ';')
		throw std::invalid_argument("Expected token ';'");
	std::string	body_size = token->substr(0, token->size() - 1);
	if (body_size == "\"\"")
		throw std::invalid_argument("Invalid index \"\"");
	if (context.top() != "server")
		throw std::invalid_argument(
			"client_body_size_limit directive is only allowd in server block");
	std::cout << "This is body size " << body_size << std::endl;
	if (server.get_is_client_body_size_set())
		throw std::invalid_argument(
			"Multiple client_body_size_limit directives is not allowed");
	if (body_size.size() > 4 
		|| (*(--body_size.end()) != 'M' && *(--body_size.end()) != 'm')
		|| !is_number(body_size.substr(0, body_size.size() - 1)) 
		|| !in_range(1, 100, atoi(body_size.c_str())))
		throw std::invalid_argument(
			"Bad argument for client_body_size_limit. Expected token "
			"between 1M and 100M");
	server.get_client_max_body_size() = atoi(body_size.c_str());
	++token;
}

static void	get_server(std::istream_iterator<std::string>& token,
						Server& server, std::stack<std::string>& context,
						const std::vector<std::string>& directives_vec,
						const std::map<std::string, std::string>& host_ip_lookup)
{
	const std::istream_iterator<std::string> end_of_file;
	if (*(++token) != "{")
		throw std::invalid_argument("Excpected token '{'");
	++token;

	int directive = -1;
	while (true)
	{
		if (*(token->begin()) == '}')
		{
			if (context.top() == "server")
			{
				std::cout << "Exiting Server block\n";
				return ;
			}
			context.pop(); /*out of the location block */
			++token;
			continue ;
		}
		else if (token == end_of_file)
			throw std::invalid_argument("Excepted token '}'");

		directive = find_directive(directives_vec, *token);
		if (directive == UNKNOWN_DIRECTIVE)
		{
			throw std::invalid_argument("Unexpected token '" + *token + "'");
			++token;
		}
		else
		{
			switch (directive)
			{
				case SERVER:
					throw std::invalid_argument("Found nested servers");
				case LISTEN:
					handle_listen(token, context, server, host_ip_lookup); break;
				case SERVER_NAME:
					handle_server_name(token, context, server); break;
				case ROOT:
					handle_root(token, context, server); break;
				case INDEX:
					handle_index(token, context, server); break;
				case AUTO_INDEX:
					handle_auto_index(token, context, server); break;
				case LOCATION:
					handle_location(token, context, server); break;
				case ERROR_DIRECTIVE:
					handle_error_directive(token, context, server); break;
				case REDIRECTION:
					handle_redirection(token, context, server); break;
				case ALLOW:
					handle_allow(token, context, server); break;
				case CLIENT_MAX_BODY_SIZE:
					handle_body_size_limit(token, context, server); break;
				default:
					throw std::invalid_argument(
						"Unexpected token '" + *token +"'");
			}
		}
	}
}

static void	set_implicit_ip_port_pairs(std::vector<Server>& servers)
{
	int i = 1;
	for (std::vector<Server>::iterator serv_it = servers.begin(); 
			serv_it != servers.end(); ++serv_it)
	{
		for (std::vector<uint16_t>::iterator it_port = 
				serv_it->get_listening_ports().begin();
				it_port != serv_it->get_listening_ports().end(); ++it_port)
		{
			for (std::map<std::string, std::string>::iterator lookup_it =
					serv_it->get_host_lookup_map()->begin(); lookup_it !=
					serv_it->get_host_lookup_map()->end(); ++lookup_it)
			{
				if (!is_in_vector(serv_it->get_implicit_port_ip_pairs(),
					std::make_pair(lookup_it->second, *it_port)))
					serv_it->get_implicit_port_ip_pairs().push_back(
						std::make_pair(lookup_it->second, *it_port));

			}
		}
		/* printing implicit ip port */
		for (std::vector<Server::ip_port_pair>::iterator imp_it = 
				serv_it->get_implicit_port_ip_pairs().begin(); imp_it != 
				serv_it->get_implicit_port_ip_pairs().end(); ++imp_it)
		{
			std::cout << "IP: " << imp_it->first << " Port: " << imp_it->second << "\n"; 
		}
		
		std::cout << "Server number " << i++ << std::endl;
	}
}

static void	init_host_ip_lookup(std::map<std::string, std::string>& host_ip_lookup)
{

	std::ifstream	hosts_file("/etc/hosts");
	if (hosts_file.fail())
		throw std::invalid_argument("Failed to open hosts file");
	
	std::string	line;
	while (std::getline(hosts_file, line))
	{
		if (std::isdigit(line[0]) && std::isdigit(line[0]) != 2)
		{
			std::istringstream tmp(line);
			std::string	ip_addr;
			std::string	host_name;
			tmp >> ip_addr;
			tmp >> host_name;
			for (std::map<std::string, std::string>::const_iterator it = 
				host_ip_lookup.begin(); it != host_ip_lookup.end(); ++it)
			{
				if (host_name == it->first)
					return ;
			}
			host_ip_lookup.insert(std::make_pair(host_name, ip_addr));
		}
	}
	hosts_file.close();
}

static void	enriche_configuration(std::vector<Server>& servers, 
									std::map<std::string, std::string>&
									host_ip_lookup)
{
	for (std::vector<Server>::iterator it = servers.begin(); 
			it != servers.end(); ++it)
	{
		it->set_host_lookup_map(&host_ip_lookup);
		if (it->get_allowed_methods().empty())
			set_default_methods(*it);
		for (size_t i = 0; i < it->get_locations().size(); ++i)
		{
			if (!it->get_locations()[i].get_allowed_methods().empty())
				set_default_methods(it->get_locations()[i]);
		}
		if (it->get_listening_ips().empty() && it->get_listening_ports().empty()
			&& it->get_ip_port_pairs().empty())
			servers.begin()->get_ip_port_pairs().push_back(
				std::make_pair("127.0.0.1", 80));
	}
	set_implicit_ip_port_pairs(servers);
}

bool	parse_config_file(const std::string& file_name, 
							std::vector<Server>& servers,
							std::map<std::string, std::string>& host_ip_lookup)
{
	init_host_ip_lookup(host_ip_lookup);
	std::ifstream	config_file(file_name.c_str());
	if (!config_file.is_open() || config_file.fail())
		throw config_file.exceptions();

	std::stack<std::string> context;
	std::istream_iterator<std::string> token(config_file);
	const std::istream_iterator<std::string> end_of_file;
	const std::vector<std::string> directives = init_directives();

	while (token != end_of_file)
	{
		if (*token == "server")
		{
			context.push("server");
			servers.push_back(Server());
			try
			{
				get_server(token, servers.back(), context, directives,
							host_ip_lookup);
				context.pop();
			}
			catch (std::exception& e)
			{
				std::cout << "Failed to parse config file\n"
						<< e.what() << "\n";
				return false;
			}
		}
		++token;
	}
	if (servers.empty())
		throw std::invalid_argument("Empty config file");
	enriche_configuration(servers, host_ip_lookup);
	config_file.close();
	return true;	
}
