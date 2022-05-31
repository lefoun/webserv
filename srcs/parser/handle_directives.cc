#include "../includes/config_parser.hpp"

void	handle_error_directive(std::istream_iterator<std::string>& token,
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
	if (token == end_of_file)
		throw std::invalid_argument("unexpected end of file");
	++token;
}

void	handle_listen(std::istream_iterator<std::string>& token,
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
		if (is_number(trimmed_token))
			set_port(trimmed_token, server);
		else
			set_ip(trimmed_token, server, host_ip_lookup);
	}
	else /* It's a host:port pair */
	{
		std::string	host = trimmed_token.substr(0, pos);
		std::string	port = trimmed_token.substr(pos + 1, std::string::npos);
		set_ip_port_pair(host, port, server, host_ip_lookup);
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
	{
		for(std::vector<std::string>::iterator it_server_name = server.get_server_names().begin();
									it_server_name != server.get_server_names().end(); ++it_server_name)
			if (*it_server_name == *token)
				throw(std::invalid_argument("found duplicate server name " + *token));
		server.get_server_names().push_back(*token);
		token++;
	}
	if (token == end_of_file)
		throw std::invalid_argument("unexpected end of file");
	for(std::vector<std::string>::iterator it_server_name = server.get_server_names().begin();
						it_server_name != server.get_server_names().end(); ++it_server_name)
		if (*it_server_name == (*token).substr(0, (*token).size() - 1))
			throw(std::invalid_argument("found duplicate server name " + *token));
	server.get_server_names().push_back(
									(*token).substr(0, (*token).size() - 1));
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

void	handle_index(std::istream_iterator<std::string>& token,
						const std::stack<std::string>& context, Server& server)
{
	check_valid_token(token);
	if (*(--(*token).end()) != ';')
		throw std::invalid_argument("Expected token ';'");
	std::string	trimmed_token = token->substr(0, token->size() - 1);
	if (trimmed_token == "\"\"")
		throw std::invalid_argument("Invalid index \"\"");
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

void	handle_location(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	check_valid_token(token);
	if (*(token->begin()) == '{') /* in case location is empty */
		throw std::invalid_argument("Expected path near location block");
	server.get_locations().push_back(Location(*token));
	++token;
	if (*(token->begin()) != '{')
		throw std::invalid_argument("Expected token '{' near location block");
	std::istream_iterator<std::string> end_of_file;
	if (++token == end_of_file)
		throw std::invalid_argument(
			"Unexpected end of file near location block");
	context.push("location");
}

void		handle_redirection(std::istream_iterator<std::string>& token,
								const std::stack<std::string>& context,
								Server& server)
{
	check_valid_token(token);
	if (*(--(*token).end()) != ';')
		throw std::invalid_argument("Expected token ';'");
	const std::string new_url = token->substr(0, token->size() - 1);
	if (context.top() == "server")
	{
		if (!server.get_redirections().empty())
			throw std::invalid_argument("Found multipe redirections in block server");
		server.get_redirections() = new_url;
	}
	else if (context.top() == "location")
	{
		if (!server.get_locations().back().get_redirections().empty())
			throw std::invalid_argument("Found multipe redirections in block location");
		server.get_locations().back().get_redirections() = new_url;
	}
	++token;
}

void	handle_allow(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	check_valid_token(token);
	std::istream_iterator<std::string>	end_of_file;
	while (*(--(token->end())) != ';' && token != end_of_file)
		set_allowed_method(*token++, context.top(), server);
	if (token == end_of_file)
		throw std::invalid_argument("Unexpected end of file");
	set_allowed_method(token->substr(0, token->size() - 1), context.top(),
						server);
	++token;
}

void	handle_body_size_limit(std::istream_iterator<std::string>& token,
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
