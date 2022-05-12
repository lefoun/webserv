#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include "Server.hpp"

enum TOKEN {
	UNKNOWN_TOKEN = -1,
	PORT = 0,
	IP,
	DOMAIN_NAME,
	PATH,
	DIRECTIVE,
	KEY_WORD,
	OPEN_BRACKET,
	CLOSED_BRACKET,
	SEMI_COLON
};

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

/* Config file parsing
 * Input is a  file
 * need to open it and have it as a stream
 * then read the stream and generate tokens out of it
 * if error return error else
 * verify if the syntaxe of the config is correct
 * if no return error
 * else create servers
 * launch main loop
*/

/* En parsant le fichier on doit trouver:
	* Directive[server] ... 
	* server : Listen 
*/

// int	get_directive(const std::string& directive)
// {
// 	return UNKNOWN_DIRECTIVE;
// }

const std::vector<std::string> init_directives()//std::vector<std::string>& directives)
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
	directives[REDIRECTION] = "redirection";
	directives[ALLOW] = "allow";
	directives[CLIENT_MAX_BODY_SIZE] = "client_max_body_size";
	return directives;
}

int	find_directive(const std::vector<std::string>& directives,
					const std::string& token)
{
	for (ssize_t i = 0; i < DIRECTIVES_NB; ++i)
		if (directives[i] == token)
			return i;
	return UNKNOWN_DIRECTIVE;
}

template <typename T>
static bool	in_range(T low, T high, T num)
{
	return num >= low && num <= high;
}

static bool is_number(const std::string& s)
{
    if(s.size() == 0)
		return false;
    for (size_t i = 0; i < s.size(); i++)
	{
        if ((s[i]>= '0' && s[i] <='9') == false)
            return false;
    }
    return true;
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
	if (!in_range(0, 65535, atoi(port_str.c_str())))
		throw std::invalid_argument("Invalid Port number");
	(server.get_listening_ports()).push_back(atoi(port_str.c_str()));
}

static bool	is_ip_address(const std::string &ip_str)
{
	if (std::count(ip_str.begin(), ip_str.end(), '.') != 3)
		return false;

	std::stringstream			ip_ss(ip_str);
	std::string					split_ip;
	std::vector<std::string>	ip_octet_holder;
	while (std::getline(ip_ss, split_ip, '.'))
	{
		ip_octet_holder.push_back(split_ip);
		std::string octect = ip_octet_holder.back();
		if (!is_number(octect) || octect.size() > 3
			|| !in_range(0, 255, atoi(octect.c_str())))
			return false;
	}
	if (ip_octet_holder.size() != 4)
		return false;
	return true;
}

static bool set_valid_host_name(const std::string& host, Server& server)
{
	std::ifstream	hosts_file(std::string("/etc/hosts"));	//open hosts file
	if (hosts_file.fail())
		throw std::invalid_argument("Failed to open hosts file");

	std::string	line;
	while (std::getline(hosts_file, line))
	{
		if (std::isdigit(line[0]) && line.find(host) != std::string::npos)
		{
			std::istringstream tmp(line);
			std::string ip_addr;
			tmp >> ip_addr;
			server.get_listening_ips().push_back(ip_addr);
			return true;
		}
	}
	return false;
}

static void	set_ip(const std::string& host, Server& server)
{
	std::ifstream	hosts_file("/etc/hosts");	//open hosts file
	if (hosts_file.fail())
		throw std::invalid_argument("Failed to open hosts file");
	
	if (is_ip_address(host))
	{
		server.get_listening_ips().push_back(host);
		return ;
	}

	if (!set_valid_host_name(host, server))
		throw std::invalid_argument("Unable to find host");
}

static void	set_allowed_method(const std::string& method,
						 const std::string& context, Server& server)
{
	if (context == "server")
		server.get_allowd_methods().push_back(method);
	else if (context == "location")
		server.get_locations().back().get_allowed_methods().push_back(method);
}

void	handle_listen(std::istream_iterator<std::string>& token,
						const std::stack<std::string>&context, Server& server)
{
	check_valid_token(token);
	if (context.top() != "server")
		throw std::invalid_argument("Unexpected token inside location block");
	if (*(--(*token).end()) != ';')	
		throw std::invalid_argument("Expected token ';'");
	std::string	trimmed_token = *token;
	trimmed_token.erase((trimmed_token.size() - 1)); /* Erase the trailing ';' */
	/* Listen either to a Host, a Port number or a Host:Port pair */
	std::string::size_type	pos = trimmed_token.find(':');
	if (pos == std::string::npos) /* Means that it's either a Host or a Port */
	{
		std::cout << "This is either a port or a host " << trimmed_token << std::endl;
		if (is_number(trimmed_token))
			set_port(trimmed_token, server);
		else
			set_ip(trimmed_token, server);
	}
	else /* It's a host:port pair */
	{
		std::cout << "This is a ip:port pair " << trimmed_token << std::endl;
		std::pair<uint16_t, std::string>	ip_port_pair;
		std::string	tmp_host = trimmed_token.substr(0, pos);
		std::string	tmp_port = trimmed_token.substr(pos + 1, std::string::npos);
		set_port(tmp_port, server);
		set_ip(tmp_host, server);
	}
	++token;
}

void	handle_server_name(std::istream_iterator<std::string>& token,
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
	server.get_server_names().push_back((*token).substr(0, (*token).size() - 1));
	++token;
}

void	handle_root(std::istream_iterator<std::string>& token,
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
			throw std::invalid_argument("Multiple Root directives is not allowed");
		server.get_root_path() = trimmed_token;
	}
	else /*the context is a location block */
	{
		if (!server.get_locations().back().get_root_path().empty())
			throw std::invalid_argument("Multiple Root directives is not allowed");
		server.get_locations().back().get_root_path() = trimmed_token;
	}
	++token;
}

void	handle_index(std::istream_iterator<std::string>& token,
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

void	handle_auto_index(std::istream_iterator<std::string>& token,
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
				"Multiple auto_index directives in server block is not allowed");
		server.get_is_auto_index_set() = true;
		if (trimmed_token == "on")
			server.get_auto_index() = true;
	}
	else if (context.top() == "location")
	{
		if (server.get_locations().back().get_is_auto_index_set())
			throw std::invalid_argument(
				"Multiple auto_index directives in location block is not allowed");
		server.get_locations().back().get_is_auto_index_set() = true;
		if (trimmed_token == "on")
			server.get_locations().back().get_auto_index() = true;
	}
	++token;
}

void	handle_location(std::istream_iterator<std::string>& token,
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

void	handle_redirection(std::istream_iterator<std::string>& token,
							const std::stack<std::string>& context,
							Server& server)
{
	(void)token; (void)context; (void)server;
}

template <typename T>
bool is_in_vector(const std::vector<T>& vect, const T& value)
{
	if (vect.empty())
		return false;
	return std::find(vect.begin(), vect.end(), value) != vect.end();
}

static void	set_error_numbers(const std::string& token,
							const std::string& context, Server& server)
{
	if (token.size() > 3 || !is_number(token)
		|| !in_range(0, 999, atoi(token.c_str())))
		throw std::invalid_argument(
			"Expected integer between 0 and 999 but got argument " + token);
	const uint16_t	error_nb = atoi(token.c_str());
	std::cout << "this is error number " << error_nb << std::endl;
	if (context == "server")
	{
		server.get_error_pages().back().second.push_back(2);
		if (is_in_vector(server.get_error_pages().back().second, error_nb))
			throw std::invalid_argument(
				"Error number " + token + " is already set in server block");
		server.get_error_pages().back().second.push_back(error_nb);
	}
	else if (context == "location")
	{
		Location *location = &(server.get_locations().back());
		if (is_in_vector(location->get_error_pages().back().second, error_nb))
			throw std::invalid_argument(
				"Error number " + token + " is already set in location block");
		location->get_error_pages().back().second.push_back(error_nb);
	}
}

void	handle_error_directive(std::istream_iterator<std::string>& token,
								const std::stack<std::string>& context,
								Server& server)
{
	check_valid_token(token);
	std::istream_iterator<std::string> end_of_file;
	while (*(--(*token).end()) != ';' && token != end_of_file)
		set_error_numbers(*token++, context.top(), server);
	const std::string error_page = token->substr(0, token->size() - 1);
	if (context.top() == "server")
		server.get_error_pages().back().first = error_page;
	else if (context.top() == "location")
		server.get_locations().back().get_error_pages().back().first = 
														error_page;
	std::cout << "This is error page " << error_page << std::endl;
	if (token == end_of_file)
		throw std::invalid_argument("unexpected end of file");
	++token;
}

void	handle_allow(std::istream_iterator<std::string>& token,
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
void	get_server(std::istream_iterator<std::string>& token, Server& server,
					std::stack<std::string>& context,
					const std::vector<std::string>& directives_vec)
{
	const std::istream_iterator<std::string> end_of_file;
	if (*(++token) != "{")
		throw std::invalid_argument("Excpected token '{'");
	++token;

	int directive;
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
		// std::cout << "This is token " << *token << std::endl;
		if (directive == UNKNOWN_DIRECTIVE)
		{
			std::cout << "Not found " << *token<< "\n";
			++token;
		}
		else
		{
			std::cout << "Found " << *token << std::endl;
			switch (directive)
			{
				case SERVER:
					throw std::invalid_argument("Found nested servers");
				case LISTEN:
					handle_listen(token, context, server); break;
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
					handle_allow(token, context, server); break;
			}
		}
	}
}

bool	parse_config_file(const std::string& file_name)
{
	std::ifstream	config_file(file_name);
	if (!config_file.is_open() || config_file.fail())
		throw config_file.exceptions();

	std::vector<Server>	servers;
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
				get_server(token, servers.back(), context, directives);
				context.pop();
			}
			catch (std::exception& e)
			{
				std::cout << "Failed to parse config file\n" << e.what() << "\n";
				return false;
			}
		}
		++token;
	}
	return false;	
}

int main()
{
	if (parse_config_file("server_config.conf"))
		std::cout << "File is good\n";
	else
		std::cout << "Error\n";
	return 0;
}
