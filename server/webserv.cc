#include "webserver.hpp"

	/*
	* sin_family = Protocol for this socket. (internet in this case)
	* sin_addr.s_addr = the IP address for this socket
	* INADDR_ANY = let the os decide
	* htons converts host to network short and htonl to long.
	*/

void	send_response()
{
	std::cout << "413 Request(entity) too large\n";
}

std::string	generate_cookie(const size_t size = 32)
{
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
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
	setenv("HTTP_COOKIE", request->cookie.c_str(), 1);
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
}

void	get_cgi_response(const request_t* request, std::string& response)
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
		// char *const args[1] = {const_cast<char *const>(request->target.c_str())};
		char *const args[1] = {NULL};
		set_cgi_env_variables(request);
		std::cout << "Executed process CGI TEST\n";
		execve("cgi-bin/cgi_test.py", args, environ);
		perror("execve failed\n");
		exit(0);
	}
	else /* parent */
	{
		wait(NULL);
		std::string	file_name = "cgi-bin/cgi_serv_communication_file.txt";
		if (!request->cookie.empty())
		{
			file_name = "cgi-bin/cookies/" + request->cookie;
		}
		std::ifstream response_file(file_name);
		if (response_file.fail())
			throw std::runtime_error("Failed to send a response from CGI");
		std::stringstream tmp;
		tmp << response_file.rdbuf();
		response_file.close();
		std::string	header_response = "HTTP/1.1 200 OK\nContent-Type:"
										" text/html\nContent-Length: ";
		response.append(header_response);
		response.append(SSTR(tmp.str().size()));
		response.append("\n\n");
		response.append(tmp.str());
	}
}

std::string	get_content_type(const std::string& file_extension)
{
	std::string content_type = " text/html";
	if (file_extension == "css")
		content_type = " text/css";
	else if (file_extension == "jpeg")
		content_type = " image/jpeg";
	else if (file_extension == "jpg")
		content_type = " image/jpg";
	else if (file_extension == "js")
		content_type = " text/javascript";
	else if (file_extension == "ico")
		content_type = " image/png";
	return content_type;
}


void	send_response(request_t* request, const int& socket_fd)
{
	std::string response;
	const std::string				file_extension = 
		request->target.substr(request->target.find_last_of(".") + 1);

	if (file_extension == "py")
	{
		std::cout << GREEN "Calling CGI Python\n" RESET;
		get_cgi_response(request, response);
	}
	else
	{
		std::string				serv_response = "HTTP/1.1 200 OK\r\nContent-Type:";
		if (request->method == "POST")
			serv_response = "HTTP/1.1 405 Not Allowed\r\nContent-Type:";
		std::string				content_type = " text/html";
		std::string				content_length = "\r\nContent-Length: ";
		unsigned int			flags = std::ios::in;

		content_type = get_content_type(file_extension);
		if (content_type.find("image", 0) != std::string::npos)
			flags = std::ios::in | std::ios::binary;

		std::string		file_path = "www/" + request->target;
		if (!request->cookie.empty() && request->target.find("html", 0) != std::string::npos)
		{
			file_path = "cgi-bin/cookies/" + request->cookie;
			if (access(file_path.c_str(), F_OK) == -1)
				file_path = "www/" + request->target;
		}
		std::ifstream 	file(file_path, flags);
		if (file.fail())
			throw std::runtime_error("Failed to open file " + file_path);
		std::ostringstream tmp_ss; 
		tmp_ss << file.rdbuf();
		std::string follow_up_rsp(tmp_ss.str());
		content_length.append(SSTR(follow_up_rsp.size()));
		serv_response.append(content_type);
		serv_response.append(content_length);
		if (request->cookie.empty() && file_extension == "html")
			serv_response.append("\nSet-Cookie: tracking-cookie="
			+ generate_cookie() + "; Expires=Tue, 03 May 2023 09:09:09 GMT");
		serv_response.append("\nConnection: keep-alive");
		serv_response.append("\n\n");
		serv_response.append(follow_up_rsp);
		file.close();
		response = serv_response;
	}
	// if (file_extension != "css")
		// std::cout << response << std::endl;
	if (send(socket_fd, response.c_str(), response.length(), 0) < 0)
		throw std::runtime_error(
			"Failed to send data to socket " + SSTR(socket_fd));
}

Server*	get_server_associated_with_request(std::vector<Server>& servers,
											const SockComm& socket,
											const char buffer[])
{
	(void)socket;
	(void)buffer;
	for (size_t i = 0; i < servers.size(); ++i)
	{
		servers[i].get_ip_port_pairs();
	}
	return NULL;
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

bool	is_complete_request(const std::string& request)
{
	read_buf(const_cast<char *>(request.c_str()), request.size());
	if (request.size() > 30)
	{
		if (request.find("GET", 0, 3) != std::string::npos)
		{
			if ((request.find(DOUBLE_CRLF, 30) != std::string::npos))
				return true;
			return false;
		}
		else if (request.find("POST", 0, 4) != std::string::npos)
		{
			std::string::size_type chunked_parsing = request.find(
				"Transfer-Encoding: chunked", 30);
			if (chunked_parsing != std::string::npos)
			{
				/* the number 57 comes from 27 which is the length of 
				 * "Transfer-Encoding: chunked" string and 30 which is
				 * the minimum number of characters in all required headers
				 * We start searching from the character 30 and 57 to gain time
				*/
				std::string::size_type pos = request.find(DOUBLE_CRLF, 57);
				if (pos != std::string::npos)
					if (request.find(DOUBLE_CRLF, pos + 4) != std::string::npos)
						return true;
				return false;
			}
			else
			{
				std::string::size_type len_pos = request.find(
												"Content-Length: ", 30);
				if (len_pos != std::string::npos)
				{
					size_t len = atoi(request.substr(len_pos + 16).c_str());
					std::string::size_type pos = request.find(
													DOUBLE_CRLF, len_pos + 16);
					if (pos != std::string::npos)
						if (request.substr(pos + 4).size() == len)
							return true;
					return false;
				}
				return false;
			}
		}
	}
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

void	launch_server(std::vector<Server>& servers,
						std::map<std::string, std::string>& host_ip_lookup)
{
	SockComm*				new_connect;
	std::vector<SockListen> listen_sockets;
	std::vector<SockComm> 	communication_sockets;
	fd_set					master_socket_list, copy_socket_list;
	char					buffer[BUFFER_SIZE + 1];
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
		if (select(fd_max_nb + 1, &copy_socket_list, NULL, NULL, NULL) == -1)
		{
			perror("Call to Select() failed"); 
			throw std::runtime_error("Call to select() failed");
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
							<< listen_sockets[index].get_port() << "\n"RESET;
					}
					catch (std::exception& e) { std::cout << e.what() << "\n"; }
				}
				else
				{
					sock_com_it_t socket_it = communication_sockets.begin()
							+ get_socket_index(communication_sockets, socket_fd);
					memset(buffer, 0, BUFFER_SIZE);
					size_t nb_bytes = recv(
						socket_fd, buffer, BUFFER_SIZE, 0);
					if (socket_it->get_client_request().empty() && nb_bytes <= 0)
					{
						close_socket(nb_bytes, master_socket_list, socket_it,
										communication_sockets);
						continue ;
					}
					socket_it->get_client_request().append(buffer);
					std::cout << "This is header\n" << 
								socket_it->get_client_request()
								<< std::endl;
					std::cout << "This is buffer size " << nb_bytes << std::endl;
					if (is_complete_request(socket_it->get_client_request()))
					{
						std::cout << BLUE "Received data from client "
									<< socket_fd << "\n"RESET;
						// if (socket_it->get_server() == NULL)
							// Server* serv = get_server_associated_with_request(
								// servers, *socket_it, buffer);
						request_t *request = get_parsed_request(
													socket_it->get_client_request());
						std::cout << BLUE "Sending data To client " << socket_fd
							<< "\n"RESET;
						send_response(request, socket_it->get_socket_fd());
						socket_it->get_client_request().clear();
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
