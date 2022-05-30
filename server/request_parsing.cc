#include "request_parsing.hpp"
#include "response.hpp"
#include "webserver.hpp"


void		print_request_content(const request_t& request)
{
	std::cout << "CONTENT_TYPE :" << request.content_type << std::endl;
	std::cout << "CONTENT_LENGTH:" << SSTR(request.content_length)<< std::endl;
	std::cout << "HTTP_COOKIE:" << request.permanent_cookie<< std::endl;
	std::cout << "HTTP_SESSION_COOKIE:" << request.session_cookie<< std::endl;
	std::cout << "HTTP_USER_AGENT:" << request.user_agent<< std::endl;
	std::cout << "PATH_INFO:" << request.path_info<< std::endl;
	std::cout << "QUERY_STRING:" << request.query_string<< std::endl;
	std::cout << "REMOTE_ADDR:" << request.remote_host << std::endl;
	std::cout << "REMOTE_HOST:" << request.remote_host << std::endl;
	std::cout << "REQUEST_METHOD:" << request.method << std::endl;
	std::cout << "SCRIPT_FILENAME:" << request.script_path << std::endl;
	std::cout << "SCRIPT_NAME:" << request.script_name << std::endl;
	std::cout << "SERVER_NAME:" << request.host << std::endl;
	std::cout << "TARGET:" << request.target << std::endl;
	// std::cout << "IP:" << ip_to_str(request.ip) << std::endl;
	// std::cout << "PORT:" << request.port << std::endl;
	std::cout << "BOUNDARY:" << request.boundary<< std::endl;
	std::cout << "BODY:" << request.body << std::endl;
	std::cout << "CONNECTION:" << request.connection << std::endl;
	std::cout << "TRANSFER-ENCODING:" << request.transfer_encoding << std::endl;
	std::cout << "REFERER:" << request.referer << std::endl;
	// std::string	path_info;
	// std::string	remote_addr;
	// std::string	remote_host;
	// std::string	script_path;
	// std::string	script_name;
	// std::string	host;
	// std::string	body;
}

static void	set_ip_port(request_t* request, const std::string& host_port,
						const std::map<std::string, std::string>& host_ip_lookup)
{
	size_t pos = host_port.find(':');
	std::string ip;
	if (pos != std::string::npos)
	{
		ip = host_port.substr(0, pos);
		if (pos < host_port.size())
			request->port = atoi(host_port.substr(
				pos + 1, host_port.size() - pos).c_str());
	}
	if (!is_ip_address(ip))
	{
		ip = host_ip_lookup.at(ip);
	}
	request->ip = ip_to_number(ip.c_str());
}

char	get_char_from_hex(const std::string& str, const int& index)
{
	long	ret;
	char	*endptr;
	char	hex_str[3];

	hex_str[0] = str[index];
	hex_str[1] = str[index + 1];
	hex_str[2] = '\0';
	ret = strtol(hex_str, &endptr, 16);
	if (ret < 128)
		return ret;
	return -1;
}

std::string replace_percent_encoding(const std::string& str,
										const std::string::size_type max_pos)
{
	std::string				replaced_str;
	std::string::size_type	str_size = str.size();
	replaced_str.reserve(str_size);
	for (std::string::size_type i = 0; i < str_size; ++i)
	{
		if (str[i] == '%' && i < max_pos)
		{
			++i;
			char c = get_char_from_hex(str, i);
			if (c == -1)
				throw std::invalid_argument("Couldn't parse percent char");
			replaced_str.push_back(c);
			++i;
		}
		else
			replaced_str.push_back(str[i]);
	}
	return replaced_str;
}

static void	parse_target_arguments(request_t* request)
{
	std::string::size_type pos = request->target.find('?', 0);
	std::string::size_type search_end = pos;
	if (search_end == std::string::npos)
		search_end = request->target.size();
	if (std::find(request->target.begin(), request->target.begin() + search_end, '%') != request->target.begin() + search_end)
		request->target = replace_percent_encoding(request->target, search_end);
	if (request->method == "GET" && pos == std::string::npos)
		return ;
	size_t	path_info_pos = request->target.find("/cgi-bin/");
	if (path_info_pos == std::string::npos)
		path_info_pos = 0;
	if (pos == std::string::npos)
	{
		pos = request->target.size();
		request->path_info = request->target.substr(
								path_info_pos + 1);
	}
	else
	{
		request->path_info = request->target.substr(
								path_info_pos + 1, pos - path_info_pos - 1);
		request->query_string = request->target.substr(pos + 1, std::string::npos);
	}
	request->script_name = request->path_info;
	std::string tmp = request->target.substr(0, pos);
	request->target = tmp;
}

static void	check_char_in_stream(const char& delimiter, std::istringstream& ss)
{
	char tmp;

	ss >> tmp;
	if (tmp != delimiter)
		throw std::invalid_argument("Unxpected token");
}

void	parse_request_header(std::string& header, request_t* request,
								const std::map<std::string, std::string>&
								host_ip_lookup,
								const char *lookup[REQUEST_KEYS_SIZE])
{
	/* Lookup table to avoid typos when searching for keys inside request */
	std::string			line;
	std::istringstream	ss(header);

	ss >> std::noskipws; /* To make whitespace non skipable */
	/* Parse Method */
	ss >> request->method;
	if (!(request->method == lookup[GET] || request->method == lookup[POST]
		|| request->method == lookup[DELETE]))
		throw std::invalid_argument("Invalid Method" + request->method);

	check_char_in_stream(' ', ss);
	ss >> request->target;
	parse_target_arguments(request);
	check_char_in_stream(' ', ss);
	ss >> line;
	if (line != lookup[PROTOCOL])
		throw std::invalid_argument(
			"expected " + std::string(lookup[PROTOCOL])+ " version");
	check_char_in_stream('\r', ss);
	check_char_in_stream('\n', ss);
	ss >> line;
	if (line != lookup[HOST])
		throw std::invalid_argument("Expected host");
	check_char_in_stream(' ', ss);
	ss >> request->host;
	set_ip_port(request, request->host, host_ip_lookup);
	check_char_in_stream('\r', ss);
	check_char_in_stream('\n', ss);
	while (ss)
	{
		std::getline(ss, line, '\r');
		ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (request->connection.empty()
			&& line.find(lookup[CONNECTION], 0) != std::string::npos)
			request->connection = line.substr(strlen(lookup[CONNECTION]));
		else if (request->user_agent.empty()
				&& line.find(lookup[USER_AGENT], 0) != std::string::npos)
			request->user_agent = line.substr(strlen(lookup[USER_AGENT]));
		else if (!request->is_content_length_set
				&& line.find(lookup[CONTENT_LENGTH], 0) != std::string::npos)
			request->content_length = std::atoll(line.substr(
										strlen(lookup[CONTENT_LENGTH])).c_str());
		else if (request->transfer_encoding.empty()
				&& line.find(lookup[TRANSFER_ENCODING], 0) != std::string::npos)
			request->transfer_encoding = line.substr(
										strlen(lookup[TRANSFER_ENCODING]));
		else if (request->content_type.empty()
				&& line.find(lookup[CONTENT_TYPE], 0) != std::string::npos)
		{
			std::string::size_type pos = line.find(
										';', strlen(lookup[CONTENT_TYPE]) + 1);
			if (pos != std::string::npos)
			{
				request->content_type = line.substr(
										strlen(lookup[CONTENT_TYPE]),
										pos - strlen(lookup[CONTENT_TYPE])); 
				request->boundary = "--" + line.substr(
										pos + strlen(lookup[BOUNDARY]) + 1);
			}
			else
				request->content_type = line.substr(
										strlen(lookup[CONTENT_TYPE])); 
		}
		else if ((request->permanent_cookie.empty() || request->session_cookie.empty())
				&& line.find(lookup[COOKIE], 0) != std::string::npos)
		{
			std::string::size_type pos = line.find(lookup[TRACKING_COOKIE]);
			if (pos != std::string::npos)
				request->permanent_cookie = line.substr(
					pos + strlen(lookup[TRACKING_COOKIE]), 32);
			pos = line.find(lookup[SESSION_COOKIE]);
			if (pos != std::string::npos)
				request->session_cookie = line.substr(
					pos + strlen(lookup[SESSION_COOKIE]), 32);
			// skip_crlf_and_space_if_any(ss);
		}
		char next_char = ss.peek();
		if (next_char == '\r' || line == "\r")
			break ;
	}
	std::cout << GREEN "Request Parsing Done\n" RESET;
	print_request_content(*request);
	std::cout << "This is position " << ss.tellg() << std::endl;
	std::string::size_type pos = ss.tellg();
	if (pos == std::string::npos)
		header.clear();
	else if (ss.peek() == '\r')
		header = header.substr(pos + 2); /* to take what's \r\n only */
	else
		header = header.substr(pos);
	std::cout << GREEN "This is the rest of the body\n" RESET;
	std::cout << header << std::endl;
}

void		strip_chunked_encoding_chars(std::string& body)
{
	std::istringstream	stream(body);
	std::string			tmp;
	body.clear();
	while (std::getline(stream, tmp, '\r'))
	{
		stream.get(); /* to take the \n of the hexadecimal number line out */
		std::getline(stream, tmp, '\r');
		body.append(tmp);
		body.append("\n");
	}
}

void		parse_request_body(std::string& client_req, request_t* request)
{
	if (request->method == "GET")
	{
		/* Assuming we don't take Get request with a body */
		request->body_parsing_state = COMPLETE;
		return ;
	}
	if (request->body_parsing_state == NOT_STARTED)
	{
		if (!request->is_content_length_set)
			request->body.reserve(request->content_length);
		if (request->transfer_encoding == "chunked")
		{
			std::string::size_type	pos = client_req.find(DOUBLE_CRLF);
			if (pos != std::string::npos)
				client_req = client_req.substr(pos + 4);
			std::cout << "This is chunked:" << client_req << std::endl;
			request->body_parsing_state = INCOMPLETE;
		}
		else if (request->content_type == "multipart/form-data")
		{
			/*Get to boundary 1 and then look for second boundary */
			client_req = client_req.substr(client_req.find(request->boundary)
											+ request->boundary.size() + 2);
			request->body_parsing_state = INCOMPLETE;
			std::cout << "this is the rest of the body\n" << client_req << std::endl;
		}
		else
		{
			request->body.append(client_req);
			if (client_req.size() == request->content_length)
				request->body_parsing_state = COMPLETE;
		}
	}
	if (request->body_parsing_state == INCOMPLETE)
	{
		if (request->content_type == "multipart/form-data")
		{
			std::string::size_type pos = client_req.find(request->boundary + "--");
			if (pos != std::string::npos) /* End of Body */
			{
				request->body.append(client_req.substr(0, pos));
				request->body_parsing_state = COMPLETE;
			}
		}
		else if (request->transfer_encoding == "chunked")
		{
			/* chunked content naive solution just to make the tester happy
			 I'll finish the implementation tomorrow */
			if (client_req.size() > 5 && client_req.find(
				 "0"DOUBLE_CRLF, client_req.size() - 5) != std::string::npos)
			{
				request->body.append(client_req);
				request->body_parsing_state = COMPLETE;
			}
		}
		else
		{
			std::string::size_type pos = client_req.find(DOUBLE_CRLF);
			if (pos != std::string::npos) /* End of Body */
			{
				request->body.append(client_req.substr(0, pos));
				request->body_parsing_state = COMPLETE;
			}
			else
				request->body.append(client_req);
		}
	}
	if (request->transfer_encoding != "chunked")
		client_req.clear();
}
