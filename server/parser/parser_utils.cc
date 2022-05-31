#include "../includes/config_parser.hpp"

int	find_directive(const std::vector<std::string>& directives,
					const std::string& token)
{
	for (ssize_t i = 0; i < DIRECTIVES_NB; ++i)
		if (directives[i] == token)
			return i;
	return UNKNOWN_DIRECTIVE;
}

void	check_valid_token(std::istream_iterator<std::string>& token)
{
	std::istream_iterator<std::string> end_of_file;
	if (++token == end_of_file)
		throw std::invalid_argument("Unexpected end of file");
	if (*(token->begin()) == ';')
		throw std::invalid_argument("Unexcpected token ';'");
}

void	set_port(const std::string& port_str, Server& server)
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

void	set_ip_port_pair(const std::string& host, const std::string& port,
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

void	set_ip(const std::string& host, Server& server,
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

void	set_allowed_method(const std::string& method,
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


void	get_server(std::istream_iterator<std::string>& token,
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

void	set_implicit_ip_port_pairs(std::vector<Server>& servers)
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
