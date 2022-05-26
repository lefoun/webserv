#include "webserver.hpp"

	/*
	* sin_family = Protocol for this socket. (internet in this case)
	* sin_addr.s_addr = the IP address for this socket
	* INADDR_ANY = let the os decide
	* htons converts host to network short and htonl to long.
	*/

size_t SockComm::session = 0;

void	send_response()
{
	std::cout << "413 Request(entity) too large\n";
}

std::string	generate_cookie(const size_t size = 32)
{
	static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(size);

	srand(time(NULL));
    for (size_t i = 0; i < size; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

    return tmp_s;
}

void	set_cgi_env_variables(const request_t* request)
{
	setenv("CONTENT_TYPE", request->method.c_str(), 1);
	setenv("CONTENT_LENGTH", request->content_length.c_str(), 1);
	setenv("HTTP_COOKIE", request->permanent_cookie.c_str(), 1);
	setenv("SESSION_COOKIE", request->session_cookie.c_str(), 1);
	setenv("HTTP_USER_AGENT", request->user_agent.c_str(), 1);
	setenv("PATH_INFO", request->path_info.c_str(), 1);
	setenv("QUERY_STRING", request->query_string.c_str(), 1);
	setenv("REMOTE_ADDR", request->remote_addr.c_str(), 1);
	setenv("REMOTE_HOST", request->remote_host.c_str(), 1);
	setenv("REQUEST_METHOD", request->method.c_str(), 1);
	setenv("SCRIPT_FILENAME", request->script_path.c_str(), 1);
	setenv("SCRIPT_NAME", request->script_name.c_str(), 1);
	setenv("SERVER_NAME", request->host.c_str(), 1);
	setenv("CONNECTION", request->connection.c_str(), 1);
	setenv("SERVER_SOFTWARE", "WebServ", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	setenv("BUFFER_SIZE", SSTR(BUFFER_SIZE).c_str(), 1);
}

void	send_chunked_response(response_t* response, std::string& response_str,
								const int& socket_fd)
{
	bool	contains_header = true;

	while (true)
	{
		char hex_num[20];
		memset(hex_num, 0, 20);
		if (response_str.size() < BUFFER_SIZE)
			sprintf(hex_num, "%lX", response_str.size());
		else
			sprintf(hex_num, "%X", BUFFER_SIZE);

		std::string chunk_len_line(std::string(hex_num) + std::string(CRLF));
		if (contains_header)
			response_str.insert(response_str.find(DOUBLE_CRLF) + 4,
								chunk_len_line); /* Insert after the header */
		else
			response_str.insert(0, chunk_len_line);

		std::string::size_type chunk_len_line_pos = response_str.find(chunk_len_line);

		std::string rsp_str;
		if (contains_header)
			 rsp_str = response_str.substr(0,
						chunk_len_line_pos + chunk_len_line.size() + BUFFER_SIZE);
		else if (response_str.size() > BUFFER_SIZE)
			rsp_str = response_str.substr(
				0, chunk_len_line.size() + BUFFER_SIZE);
		else
			 rsp_str = response_str.substr(
				 0, chunk_len_line.size() + response_str.size());

		if (rsp_str.size() < BUFFER_SIZE)
		{
			rsp_str.append("\r\n0\r\n\r\n"); /* End of response */
			response->response_state = COMPLETE;
		}
		else
			rsp_str.append("\r\n");

		// std::cout << "response to send:\n" << rsp_str << std::endl;

		if (send(socket_fd, rsp_str.c_str(), rsp_str.size(), 0) < 0)
			throw std::runtime_error("Failed to send data to socket " +
										SSTR(socket_fd));
		if (response->response_state == COMPLETE)
			return ;
		if (contains_header)
			response_str = response_str.substr(
				chunk_len_line_pos + chunk_len_line.size() + BUFFER_SIZE);
		else
			response_str = response_str.substr(BUFFER_SIZE);
		contains_header = false;
	}
}

void	get_cgi_response(const request_t* request, response_t* response,
							std::string& response_str, const int& socket_fd)
{
	pid_t child_pid = 1;

	child_pid = fork();

	if (child_pid < 0)
	{
		std::cout << "Failed to create a new process\n";
		return ;
	}
	if (child_pid == 0) /* Child process */
	{
		extern char **environ;
		char *args[3];
		args[0] = const_cast<char *const>(request->path_info.c_str());
		args[1] = const_cast<char *const>(request->body.c_str());
		args[2] = NULL;
		set_cgi_env_variables(request);
		std::cout << "Executed process CGI TEST\n";
		std::cout << request->path_info << std::endl;
		execve(*args, args + 1, environ);
		perror("execve failed\n");
		exit(0);
	}
	else /* parent */
	{
		wait(NULL);
		std::string	file_name = "cgi-bin/cgi_serv_communication_file.txt";
		std::ifstream cgi_output_file(file_name.c_str());
		if (cgi_output_file.fail())
			throw std::runtime_error("Failed to send a response from CGI");
		std::stringstream tmp;
		tmp << cgi_output_file.rdbuf();
		cgi_output_file.close();
		response_str = tmp.str();
		if (response_str.size() >= BUFFER_SIZE)
		{
			send_chunked_response(response, response_str, socket_fd);
			return ;
		}
		if (send(socket_fd, response_str.c_str(), response_str.size(), 0) < 0)
			throw std::runtime_error(
				"Failed to send data to socket " + SSTR(socket_fd));
	}
}
/*
 * Response: Header + Body(optional)
 *
*/

std::string	get_content_type(const std::string& file_extension)
{
	std::string content_type = "text/html";
	if (file_extension == "css")
		content_type = "text/css";
	else if (file_extension == "jpeg")
		content_type = "image/jpeg";
	else if (file_extension == "jpg")
		content_type = "image/jpg";
	else if (file_extension == "js")
		content_type = "text/javascript";
	else if (file_extension == "ico")
		content_type = "image/png";
	return content_type;
}


void	construct_header(response_t* response, request_t* request,
							std::string& header)
{
	header.reserve(100);
	header.append("HTTP/1.1 ");
	header.append(SSTR(response->return_code));
	header.append(" ");
	header.append(response->return_message + CRLF);
	if (response->return_code != 302)
	{
		std::string content_type = get_content_type(
			request->target.substr(request->target.find_last_of(".") + 1));
		header.append("content-type: " );
		response->content_type = content_type;
		header.append(content_type.append(CRLF));
		header.append("Connection: keep-alive\r\n");
	}
	if (!response->is_chunked)
	{
		header.append("content-length: ");
		header.append(SSTR(response->body.size()) + CRLF);
	}
	if (response->is_chunked)
		header.append("Transfer-Encoding: chunked\r\n");
	header.append("date: ");
	header.append(response->date.append(CRLF));
	if (response->return_code == 302)
	{
		header.append("location: ");
		header.append(response->location.append(CRLF));
	}
	if (request->permanent_cookie.empty() && response->content_type == "text/html")
	{
		header.append("Set-Cookie: tracking-cookie=" + generate_cookie() + "; Expires=" + get_current_time(1));
		header.append(CRLF);
		std::cout << "Appending time\n\n";
	}
	header.append(CRLF);
}

void	send_response(request_t* request, const int& socket_fd,
						response_t* response)
{
	std::string	response_str;
	if (response->response_state == COMPLETE)
	{
		construct_header(response, request, response_str);
		if (!response->body.empty())
			response_str.append(response->body);
		if (send(socket_fd, response_str.c_str(), response_str.size(), 0) < 0)
			throw std::runtime_error(
				"Failed to send data to socket " + SSTR(socket_fd));
	}
	else /* Not started all Get/Post/Delete Requests that are valid (till now) */
	{
		std::string file_extension = request->target.substr(
									request->target.find_last_of(".") + 1);
		if (file_extension == "py" || file_extension == "php")
		{
			std::cout << GREEN "Calling CGI " + file_extension + "\n" RESET;
			get_cgi_response(request, response, response_str, socket_fd);
			return ;
		}
		else
		{
			if (file_extension == "html" && !request->session_cookie.empty())
			{
				std::string cookie_file_path = "cgi-bin/cookies/"
												+ request->session_cookie + "_form";
				if (access(cookie_file_path.c_str(), R_OK) == -1)
					perror(cookie_file_path.c_str());
				else
					response->file_path = cookie_file_path;
			}
		}
		std::ifstream	file;
		if (response->content_type.find("image", 0) != std::string::npos)
			file.open(response->file_path.c_str(), std::ios::in | std::ios::binary);
		else
			file.open(response->file_path.c_str(), std::ios::in);
		if (file.fail())
			throw std::runtime_error("Failed to open file " + response->file_path);
		std::ostringstream tmp_ss;
		tmp_ss << file.rdbuf();
		response->body = tmp_ss.str();
		if (response->body.size() > BUFFER_SIZE)
		{
			response->is_chunked = true;
			construct_header(response, request, response_str);
			response_str.append(tmp_ss.str());
			send_chunked_response(response, response_str, socket_fd);
			file.close();
			return ;
		}
		construct_header(response, request, response_str);
		response_str.append(response->body);
		if (send(socket_fd, response_str.c_str(), response_str.size(), 0) < 0)
			throw std::runtime_error(
				"Failed to send data to socket " + SSTR(socket_fd));
		file.close();
	}
}

Server*	get_server_associated_with_request(std::vector<Server>& servers,
											const request_t *request)
{
	const std::map<std::string, std::string>&	host_ip_lookup = *(servers[0].get_host_lookup_map());
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
		std::cout << "ip=" << it->get_listening_ips().back()  << std::endl;
		std::vector<Server::ip_port_pair>::iterator it_explicit_ip_port = it->get_ip_port_pairs().begin();
		while (it->get_ip_port_pairs().size() && it_explicit_ip_port != it->get_ip_port_pairs().end())
		{
			if (it_explicit_ip_port->first == ip && it_explicit_ip_port->second == port)
				return &(*it);

			++it_explicit_ip_port;
		}
		std::vector<std::string>::iterator it_listening_ip = it->get_listening_ips().begin();
		while (it->get_listening_ips().size() && it_listening_ip != it->get_listening_ips().end())
		{
			if (*it_listening_ip == ip && port == 8000)
				return &(*it);
			++it_listening_ip;
		}
		std::vector<Server::ip_port_pair>::iterator it_implicit_ip_port = it->get_implicit_port_ip_pairs().begin();
		while (it->get_implicit_port_ip_pairs().size() &&  it_implicit_ip_port != it->get_implicit_port_ip_pairs().end())
		{
			if (it_implicit_ip_port->first == ip && it_implicit_ip_port->second == port)
			{
				if (!associated_serv)
					associated_serv = &(*it);
				else if (!host.empty())
				{
					std::vector<std::string>::iterator it_server_names = it->get_server_names().begin();
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

int	return_error(const std::string& error_msg)
{
	perror(error_msg.c_str());
	return errno;
}

template <typename T>
int get_socket_index(const std::vector<T>& vec, const int& socket)
{
	for (size_t i = 0; i < vec.size(); ++i)
	{
		if (vec[i].get_socket_fd() == socket)
			return i;
	}
	return -1;
}

template <typename U>
bool	is_socket_already_open(const U vect, uint16_t port, in_addr_t ip)
{
	for (size_t i = 0; i < vect.size(); ++i)
	{
		if (vect[i].get_ip() == 0 && vect[i].get_port() == port)
			return true;
		else if (port == vect[i].get_port() && ip == vect[i].get_ip())
			return true;
	}
	return false;
}

void	open_listening_sockets(std::vector<SockListen>& sockets,
								std::vector<Server>& servers)
{
	/* find listening ports */
	for (std::vector<Server>::iterator it = servers.begin();
			it != servers.end(); ++it)
	{
		for (std::vector<uint16_t>::iterator it_port =
				it->get_listening_ports().begin();
				it_port != it->get_listening_ports().end(); ++it_port)
		{
			if (!is_socket_already_open(sockets, *it_port, INADDR_ANY))
			{
				std::cout << "Opening Socket PORT: " << *it_port
							<< " IP: ALL\n";
				sockets.push_back(SockListen(*it_port, INADDR_ANY));
			}
		}
	}

	/* find listening ips with default port = 80*/
	for (std::vector<Server>::iterator it = servers.begin();
			it != servers.end(); ++it)
	{
		for (std::vector<std::string>::iterator it_ip =
				it->get_listening_ips().begin();
				it_ip!= it->get_listening_ips().end(); ++it_ip)
		{
			if (!is_socket_already_open(sockets, 8000,
				ip_to_number(it_ip->c_str())))
			{
				std::cout << "Opening Socket PORT: 8000"
							<< " IP: " << *it_ip << "\n";
				sockets.push_back(
					SockListen(8000, ip_to_number(it_ip->c_str())));
			}
		}
	}

	/* find listening pairs Ip:port */
	for (std::vector<Server>::iterator it = servers.begin();
			it != servers.end(); ++it)
	{
		for (std::vector<Server::ip_port_pair>::iterator it_pair =
				it->get_ip_port_pairs().begin();
				it_pair!= it->get_ip_port_pairs().end(); ++it_pair)
		{
			if (it_pair->second != 0
				&& !is_socket_already_open(sockets, it_pair->second,
				ip_to_number(it_pair->first.c_str())))
			{
				std::cout << "Opening Socket pair PORT: "
							<< it_pair->second
							<< " IP: " << it_pair->first << "\n";
				sockets.push_back(SockListen(it_pair->second,
									ip_to_number(it_pair->first.c_str())));
			}
		}
	}
}

void	bind_sockets(std::vector<SockListen>& listen_sockets,
						fd_set& master_socket_list)
{
	for (std::vector<SockListen>::iterator it = listen_sockets.begin();
		it != listen_sockets.end(); ++it)
	{
		it->bind_socket();
		it->listen_socket();
		FD_SET(it->get_socket_fd(), &master_socket_list);
		std::cout << BLUE "Listening on socket " << it->get_socket_fd()
				<< " and port "<< it->get_port()
				<< " and ip " << inet_ntoa(it->get_sockaddr_in().sin_addr)
				<< "\n" RESET;
	}
}

bool	is_complete_request(std::string& request, request_t *rqst,
							const std::map<std::string, std::string>&
							host_ip_lookup)
{
	// read_buf(const_cast<char *>(request.c_str()), request.size());
	if (rqst->method.empty()) /* Request header is not parsed yet */
	{
		if (request.find(DOUBLE_CRLF) != std::string::npos)
		{
			/* parse_request_body:
			 * Parses the request and put teh values in the struct rqst and
			 * trunks the request string to leave only the body */
			parse_request_header(request, rqst, host_ip_lookup);
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

void	close_socket(const ssize_t nb_bytes, fd_set& master_socket_list,
						sock_com_it_t& socket_it, std::vector<SockComm>&
						communication_sockets)
{
	if (nb_bytes == 0)
		std::cout << RED"Connection closed from Socket "
					<< socket_it->get_socket_fd() << RESET << std::endl;
	else if (nb_bytes < 0)
		perror("Recv failed");
	FD_CLR(socket_it->get_socket_fd(), &master_socket_list);
	communication_sockets.erase(socket_it);
	socket_it->close_socket();
}

void clear_request(request_t & request)
{
	request.content_type.clear();
	request.content_length.clear();
	request.user_agent.clear();
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

void	launch_server(std::vector<Server>& servers,
						std::map<std::string, std::string>& host_ip_lookup)
{
	SockComm*				new_connect;
	std::vector<SockListen> listen_sockets;
	std::vector<SockComm> 	communication_sockets;
	fd_set					master_socket_list, copy_socket_list;
	char					buffer[BUFFER_SIZE];
	int						index = 0;

	(void)host_ip_lookup;
	/* creates the sockets */
	FD_ZERO(&master_socket_list);
	FD_ZERO(&copy_socket_list);
	open_listening_sockets(listen_sockets, servers);
	bind_sockets(listen_sockets, master_socket_list);
	int	fd_max_nb = listen_sockets.back().get_socket_fd();
	while (true)
	{
		std::cout << MAGENTA "====== Waiting for incoming new connections "
						<< "======\n" RESET;
		/* Copy our original socket list into copy because Select() will erase
		 * its contents and leave only read-ready sockets */
		copy_socket_list = master_socket_list;

		// accept incoming connections
		try
		{
			if (select(fd_max_nb + 1, &copy_socket_list, NULL, NULL, NULL) == -1)
			{
				perror("Call to Select() failed");
				throw std::runtime_error("Call to select() failed");
			}
		}
		catch (std::exception & e)
		{
			std::cerr << RED "Error: " << e.what() << RESET << std::endl;
		}
		for (int socket_fd = 0; socket_fd <= fd_max_nb; ++socket_fd)
		{
			if (FD_ISSET(socket_fd, &copy_socket_list))
			{
				index = get_socket_index(listen_sockets, socket_fd);
				if (index != -1)
				{
					try
					{
						new_connect = listen_sockets[index].accept_connection();
						std::cout << "Before closing any socket\n";
						communication_sockets.push_back(*new_connect);
						FD_SET(new_connect->get_socket_fd(), &master_socket_list);
						if (new_connect->get_socket_fd() > fd_max_nb)
							fd_max_nb = new_connect->get_socket_fd();
						std::cout <<
							GREEN "Server Accepted new connection on socket "
							<< listen_sockets[index].get_port() << "\n" RESET;
					}
					catch (std::exception& e) { std::cout << e.what() << "\n"; }
				}
				else
				{
					sock_com_it_t socket_it = communication_sockets.begin()
							+ get_socket_index(communication_sockets, socket_fd);
					memset(buffer, 0, BUFFER_SIZE);
					int nb_bytes = recv(
						socket_fd, buffer, BUFFER_SIZE, 0);
					if (socket_it->get_client_request().empty() && nb_bytes <= 0)
					{
						close_socket(nb_bytes, master_socket_list, socket_it,
										communication_sockets);
						continue ;
					}
					if (nb_bytes > -1)
						socket_it->get_client_request().append(buffer, nb_bytes);
					std::cout << "This is header\n" <<
								socket_it->get_client_request()
								<< std::endl;
					std::cout << "This is buffer size " << nb_bytes << std::endl;
					std::cout << BLUE "Received data from client "
								<< socket_fd << "\n"RESET;
					if (is_complete_request(socket_it->get_client_request(),
											&socket_it->get_request(),
											host_ip_lookup))
					{
						std::cout << BLUE "Sending data To client " << socket_fd
							<< "\n"RESET;
						if (socket_it->get_request().transfer_encoding != "chunked"
							|| socket_it->get_request().body_parsing_state == COMPLETE)
						{
							Server* serv = get_server_associated_with_request(servers, &socket_it->get_request());
							std::cout << "chosen server = " << serv->get_server_names().back() << std::endl;
							if (serv == NULL)
								std::cout << "NULL" << std::endl;
							try
							{
								set_response(*serv, &socket_it->get_request(),
													 &socket_it->get_response());
								send_response(&socket_it->get_request(),
												socket_it->get_socket_fd(),
												&socket_it->get_response());
							}
							catch (std::exception& e) { std::cout << e.what() << "\n"; }
							socket_it->get_client_request().clear();
							clear_request(socket_it->get_request());
							socket_it->get_request().body_parsing_state = NOT_STARTED;
						}
					}
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	std::vector<Server>					servers;
	std::map<std::string, std::string>	host_ip_lookup;
	std::string							config_file;
	if (argc < 2)
		config_file = "server_config.conf";
	else
		config_file = argv[0];
	try
	{
		parse_config_file(config_file, servers, host_ip_lookup);
		std::cout << GREEN "Loaded config file.\n\n"
					<< "Starting WebServer...\n"RESET;
		launch_server(servers, host_ip_lookup);
		return 0;
	}
	catch (std::exception &e) { std::cout << e.what() << std::endl; }
	return 1;
}
