#include "request_parsing.hpp"
#include "webserver.hpp"

void		print_request_content(const request_t& request)
{
	std::cout << "CONTENT_TYPE :" << request.content_type << std::endl;
	std::cout << "CONTENT_LENGTH:" << request.content_length<< std::endl;
	std::cout << "HTTP_COOKIE:" << request.cookie<< std::endl;
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
	// std::string	path_info;
	// std::string	remote_addr;
	// std::string	remote_host;
	// std::string	script_path;
	// std::string	script_name;
	// std::string	host;
	// std::string	body;
}

static void	parse_target_arguments(request_t* request)
{
	size_t pos = request->target.find('?', 0);	
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
}

void	parse_request_header(std::string& header, request_t* request)
{
	/* Lookup table to avoid typos when searching for keys inside request */
	static const char *lookup[REQUEST_KEYS_SIZE];
	lookup[GET] = "GET";
	lookup[POST] = "POST";
	lookup[DELETE] = "DELETE";
	lookup[PROTOCOL] = "HTTP/1.1";
	lookup[HOST] = "Host:";
	lookup[COOKIE] = "Cookie: ";
	lookup[CONNECTION] = "Connection: ";
	lookup[USER_AGENT] = "User-Agent: ";
	lookup[CONTENT_LENGTH] = "Content-Length: ";
	lookup[CONTENT_TYPE] = "Content-Type: ";
	lookup[TRACKING_COOKIE] = "tracking-cookie=";
	lookup[BOUNDARY] = " boundary=";
	lookup[TRANSFER_ENCODING] = "Transfer-Encoding: ";
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
	check_char_in_stream('\r', ss);
	check_char_in_stream('\n', ss);
	while (ss)
	{
		std::getline(ss, line, '\r');
		if (request->connection.empty()
			&& line.find(lookup[CONNECTION], 0) != std::string::npos)
			request->connection = line.substr(strlen(lookup[CONNECTION]));
		else if (request->user_agent.empty()
				&& line.find(lookup[USER_AGENT], 0) != std::string::npos)
			request->user_agent = line.substr(strlen(lookup[USER_AGENT]));
		else if (request->content_length.empty()
				&& line.find(lookup[CONTENT_LENGTH], 0) != std::string::npos)
			request->content_length = line.substr(
										strlen(lookup[CONTENT_LENGTH]));
		else if (request->transfer_encoding.empty()
				&& line.find(lookup[TRANSFER_ENCODING], 0) != std::string::npos)
			request->transfer_encoding= line.substr(
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
		else if (request->cookie.empty()
				&& line.find(lookup[COOKIE], 0) != std::string::npos)
		{
			std::string::size_type pos = line.find(lookup[TRACKING_COOKIE]);
			if (pos != std::string::npos)
				request->cookie = line.substr(
					pos + strlen(lookup[TRACKING_COOKIE]), 32);
			skip_crlf_and_space_if_any(ss);
		}
		std::cout << "This is get " << ss.get() << std::endl;
		if (ss.peek() == '\r')
			break ;
	}
	std::cout << GREEN "Request Parsing Done\n" RESET;
	print_request_content(*request);
	std::cout << "This is position " << ss.tellg() << std::endl;
	std::string::size_type pos = ss.tellg();
	if (pos == std::string::npos)
		header.clear();
	else
		header = header.substr(ss.tellg());
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
		if (!request->content_length.empty())
			request->body.reserve(request->content_length.size());
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
			if (client_req.size() == request->content_length.size())
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

request_t*	get_parsed_request(std::string& header)
{
	/* check request
	 * HOSTNAME:PORT
	 * PORT
	 * IP
	 * server_name
	*/
	request_t	*request = new request_t;
	parse_request_header(header, request);
	// request->path_info = 
	return request;
}
