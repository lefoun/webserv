#include "request_parsing.hpp"

void		print_request_content(const request_t& request)
{
	std::cout << "CONTENT_TYPE :" << request.content_type << std::endl;
	std::cout << "CONTENT_LENGTH:" << request.content_length<< std::endl;
	std::cout << "HTTP_COOKIE:" << request.cookie<< std::endl;
	std::cout << "HTTP_USER_AGENT:" << request.user_agent<< std::endl;
	std::cout << "PATH:" << request.path_info<< std::endl;
	std::cout << "QUERY_STRING:" << request.query_string<< std::endl;
	std::cout << "REMOTE_ADDR:" << request.content_type << std::endl;
	std::cout << "REMOTE_HOST:" << request.remote_host << std::endl;
	std::cout << "REQUEST_METHOD:" << request.method << std::endl;
	std::cout << "SCRIPT_FILENAME:" << request.script_path << std::endl;
	std::cout << "SCRIPT_NAME:" << request.script_name << std::endl;
	std::cout << "SERVER_NAME:" << request.host << std::endl;
	std::cout << "TARGET:" << request.target << std::endl;
	// std::cout << "IP:" << ip_to_str(request.ip) << std::endl;
	// std::cout << "PORT:" << request.port << std::endl;
	std::cout << "BODY:" << request.body << std::endl;
	std::cout << "CONNECTION:" << request.connection << std::endl;
	// std::string	path_info;
	// std::string	remote_addr;
	// std::string	remote_host;
	// std::string	script_path;
	// std::string	script_name;
	// std::string	host;
	// std::string	body;
}

static void	parse_target_arguments(request_t& request)
{
	size_t pos = request.target.find('?', 0);	
	if (pos == std::string::npos)
		return ;
	request.query_string = request.target.substr(pos + 1, std::string::npos);
	std::string tmp = request.target.substr(0, pos);
	request.target = tmp;
}

static void	check_char_in_stream(const char& delimiter, std::istringstream& ss)
{
	char tmp;

	ss >> tmp;
	if (tmp != delimiter)
		throw std::invalid_argument("Unxpected token");
}

static void	skip_crlf_and_space_if_any(std::istringstream& ss)
{
	char c;
	c = ss.peek();
	if (c == '\r')
	{
		ss >> c;
		ss >> c;
	}
	else if (c == '\n' || c == ' ')
		ss >> c;
	std::cout << "This is c " << int(c) << std::endl;
}

static void	parse_request_header(const std::string& header, request_t& request)
{
	/* Request format
	 * request-line = method SP request-target SP HTTP-version CRLF
	 */

	std::string			tmp;
	std::istringstream	ss(header);

	ss >> std::noskipws; /* To make whitespace non skipable */
	/* Parse Method */
	ss >> request.method;
	if (!(request.method == "GET" || request.method == "POST"
		|| request.method == "DELETE"))
		throw std::invalid_argument("Invalid Method" + request.method);

	check_char_in_stream(' ', ss);
	ss >> request.target;
	parse_target_arguments(request);
	check_char_in_stream(' ', ss);
	ss >> tmp;
	if (tmp != "HTTP/1.1")
		throw std::invalid_argument("expected HTTP/1.1 version");
	check_char_in_stream('\r', ss);
	check_char_in_stream('\n', ss);
	ss >> tmp;
	if (tmp != "Host:")
		throw std::invalid_argument("Expected host");
	check_char_in_stream(' ', ss);
	ss >> request.host;
	check_char_in_stream('\r', ss);
	check_char_in_stream('\n', ss);
	std::cout << "This is request\n" << header << std::endl;
	while (ss)
	{
		ss >> tmp;
		skip_crlf_and_space_if_any(ss);
		// std::cout << "This is tmp " + tmp << std::endl;
		if (tmp == "User-Agent:")
			ss >> request.user_agent;
		else if (tmp == "Connection:")
			ss >> request.connection;
		else if (tmp == "Content-Length:")
			ss >> request.content_length;
		else if (tmp == "Content-Type:")
			ss >> request.content_type;
		else if (tmp == "Cookie:")
		{
			std::string::size_type pos = header.find("tracking-cookie=", ss.tellg());
			if (pos != std::string::npos)
				request.cookie = header.substr(pos + sizeof("tracking-cookie"), 32);
		}
		skip_crlf_and_space_if_any(ss);
		if (ss.peek() == '\r')
			break ;
	}
	if (ss)
	{
		skip_crlf_and_space_if_any(ss);
		request.body = header.substr(ss.tellg());
	}
	std::cout << GREEN "Request Parsing Done\n" RESET;
	print_request_content(request);
}

request_t*	get_parsed_request(const std::string& header)
{
	/* check request
	 * HOSTNAME:PORT
	 * PORT
	 * IP
	 * server_name
	*/
	request_t	*request = new request_t;
	parse_request_header(header, *request);
	return request;
}
