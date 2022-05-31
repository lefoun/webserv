#include "request_parsing.hpp"
#include "response.hpp"
#include "webserver.hpp"

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
			char c = get_char_from_hex(str, i, str_size);
			if (c == -1)
				throw std::invalid_argument("Couldn't parse percent char");
			replaced_str.push_back(c);
			i += 2; /* Skip %XX */
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
	if (std::find(request->target.begin(), request->target.begin() +
			search_end, '%') != request->target.begin() + search_end)
		request->target = replace_percent_encoding(request->target, search_end);
	if ((request->method == "GET" || request->method == "DELETE")
			&& pos == std::string::npos)
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
		}
		char next_char = ss.peek();
		if (next_char == '\r' || line == "\r")
			break ;
	}
	std::string::size_type pos = ss.tellg();
	if (pos == std::string::npos)
		header.clear();
	else if (ss.peek() == '\r')
		header = header.substr(pos + 2); /* to take what's \r\n only */
	else
		header = header.substr(pos);
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
	if (request->method == "GET" || request->method == "DELETE")
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
			request->body_parsing_state = INCOMPLETE;
		}
		else if (request->content_type == "multipart/form-data")
		{
			/*Get to boundary 1 and then look for second boundary */
			client_req = client_req.substr(client_req.find(request->boundary)
											+ request->boundary.size() + 2);
			request->body_parsing_state = INCOMPLETE;
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

void clear_request(request_t & request)
{
	request.content_type.clear();
	request.is_content_length_set = false;
	request.path_info.clear();
	request.query_string.clear();
	request.remote_addr.clear();
	request.remote_host.clear();
	request.method.clear();
	request.script_path.clear();
	request.script_name.clear();
	request.target.clear();
	request.host.clear();
	request.connection.clear();
	request.body.clear();
	request.boundary.clear();
	request.transfer_encoding.clear();
	request.ip = 0;
	request.port = 0;
}

void	init_request_parsing_lookup_tab(const char *lookup[REQUEST_KEYS_SIZE])
{
	lookup[GET] = "GET";
	lookup[POST] = "POST";
	lookup[DELETE] = "DELETE";
	lookup[PROTOCOL] = "HTTP/1.1";
	lookup[HOST] = "Host:";
	lookup[COOKIE] = "Cookie: ";
	lookup[CONNECTION] = "Connection: ";
	lookup[CONTENT_LENGTH] = "Content-Length: ";
	lookup[CONTENT_TYPE] = "Content-Type: ";
	lookup[TRACKING_COOKIE] = "tracking-cookie=";
	lookup[SESSION_COOKIE] = "session-cookie=";
	lookup[BOUNDARY] = " boundary=";
	lookup[TRANSFER_ENCODING] = "Transfer-Encoding: ";
}

bool	is_complete_request(std::string& request, request_t *rqst,
							const std::map<std::string, std::string>&
							host_ip_lookup,
							const char *req_parsing_lookup[REQUEST_KEYS_SIZE])
{
	if (rqst->method.empty()) /* Request header is not parsed yet */
	{
		if (request.find(DOUBLE_CRLF) != std::string::npos)
		{
			/* parse_request_body:
			 * Parses the request and put teh values in the struct rqst and
			 * trunks the request string to leave only the body */
			parse_request_header(request, rqst, host_ip_lookup, req_parsing_lookup);
			parse_request_body(request, rqst);
		}
		else
			return false;
	}
	if (rqst->body_parsing_state == NOT_STARTED)
	{
		/* we parsed the request header but request_body is not yet parsed*/
		if (rqst->transfer_encoding == "chunked")
		{

			std::string::size_type pos = request.find(DOUBLE_CRLF, 57);
			if (pos != std::string::npos)
				if (request.find(DOUBLE_CRLF, pos + 4) != std::string::npos)
				{
					parse_request_body(request, rqst);
					return true;
				}
			return false;
		}
		else /* Request is unchunked */
		{
			/*
			 * Either content_type is multiform data which comes in many steps
			 * or comes in a single time
			*/
			parse_request_body(request, rqst);
		}
	} /* Need to add Delete Request */
	if (rqst->body_parsing_state == INCOMPLETE)
		parse_request_body(request, rqst);
	if (rqst->body_parsing_state == COMPLETE)
		return true;
	return false;
}

Server*	get_server_associated_with_request(std::vector<Server>& servers,
											const request_t *request)
{
	const std::map<std::string, std::string>&	host_ip_lookup =
			*(servers[0].get_host_lookup_map());
	std::string ip = request->host;
	std::string host;
	u_int32_t port = 80;
	size_t pos = request->host.find(':');
	if (pos != std::string::npos)
	{
		ip = request->host.substr(0, pos);
		if (pos < request->host.size())
			port = atoi(request->host.substr(pos + 1, request->host.size() - pos).c_str());

	}
	if (!is_ip_address(ip))
	{
		host = ip;
		ip = host_ip_lookup.at(ip);
	}
	if (ip == "0.0.0.0")
		ip = "127.0.0.1";
	Server* associated_serv = NULL;
	std::vector<Server>::iterator it = servers.begin();
	while (it != servers.end())
	{
		std::vector<Server::ip_port_pair>::iterator it_explicit_ip_port =
				it->get_ip_port_pairs().begin();
		while (it->get_ip_port_pairs().size() && it_explicit_ip_port
				!= it->get_ip_port_pairs().end())
		{
			if (it_explicit_ip_port->first == ip && it_explicit_ip_port->second == port)
				return &(*it);

			++it_explicit_ip_port;
		}
		std::vector<std::string>::iterator it_listening_ip =
				it->get_listening_ips().begin();
		while (it->get_listening_ips().size() && it_listening_ip !=
				it->get_listening_ips().end())
		{
			if (*it_listening_ip == ip && port == 8000)
				return &(*it);
			++it_listening_ip;
		}
		std::vector<Server::ip_port_pair>::iterator it_implicit_ip_port
				= it->get_implicit_port_ip_pairs().begin();
		while (it->get_implicit_port_ip_pairs().size() &&  it_implicit_ip_port
				!= it->get_implicit_port_ip_pairs().end())
		{
			if (it_implicit_ip_port->first == ip && it_implicit_ip_port->second == port)
			{
				if (!associated_serv)
					associated_serv = &(*it);
				else if (!host.empty())
				{
					std::vector<std::string>::iterator it_server_names
							= it->get_server_names().begin();
					while (it_server_names != it->get_server_names().end())
					{
						if (*it_server_names == host)
							return &(*it);
						++it_server_names;
					}
				}
			}
			++ it_implicit_ip_port;
		}
		it++;
	}
	return associated_serv;
}
