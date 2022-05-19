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

void	get_cgi_response(request_t* request, std::string& response)
{
	pid_t child_pid = fork();
	char	*arg[1];
	arg[0] = NULL;

	if (child_pid < 0)
	{
		std::cout << "Failed to create a new process\n";
		return ;
	}
	if (child_pid == 0) /* Child process */
	{
		extern char **environ;
		setenv("QUERY_STRING", request->args.c_str(), 1);
		execve("../cgi-bin/cgi_test.py", arg, environ);
		std::cout << "Executed process CGI TEST\n";
		exit(0);
	}
	else /* parent */
	{
		wait(NULL);
		std::ifstream response_file("cgi-bin/cgi_serv_communication_file.txt");
		if (response_file.fail())
			throw std::runtime_error("Failed to send a response from CGI");
		std::stringstream tmp;
		tmp << response_file.rdbuf();
		response_file.close();
		std::string	header_response = "HTTP/1.1 200 OK\nContent-Type:"
										" text/html\nContent-Length: ";
		response.append(header_response);
		response.append(SSTR(tmp.str().size()));
		response.append("\nConnection: keep-alive\n");
		response.append("\n\n");
		response.append(tmp.str());
	}
}

void	send_response(request_t* request, const int& socket_fd)
{
	std::string response;

	if (request->method == "GET" && !request->args.empty() && request->target.find_last_of(".py") != std::string::npos)
	{
		std::cout << GREEN "Calling CGI Python\n" RESET;
		get_cgi_response(request, response);
	}
	else
	{
		std::string				serv_response = "HTTP/1.1 200 OK\nContent-Type:"
											" text/html\nContent-Length: ";
		std::ifstream html_form("www/form.html");
		if (html_form.fail())
			throw std::runtime_error("Failed to open file form.html");
		std::stringstream tmp_ss; 
		tmp_ss << html_form.rdbuf();
		serv_response.append(SSTR(tmp_ss.str().size()));
		std::string follow_up_rsp = "\n\n" + tmp_ss.str();
		serv_response.append("\nConnection: keep-alive\n");
		serv_response.append(follow_up_rsp);
		html_form.close();
		response = serv_response;
		std::cout << "Sending this response from else: " << response << std::endl;
	}
	std::cout << response << std::endl;
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
int get_socket_index(const std::vector<T>& vec, int socket)
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
		if (port == vect[i].get_port() && ip == vect[i].get_ip())
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
				std::cout << BLUE "Opening Socket PORT: " << *it_port
							<< " IP: ALL\n" RESET;
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
				std::cout << BLUE "Opening Socket PORT: 8000"
							<< " IP: " << *it_ip << "\n" RESET;
				sockets.push_back(SockListen(8000, ip_to_number(it_ip->c_str())));
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
				std::cout << BLUE "Opening Socket pair PORT: " << it_pair->second
							<< " IP: " << it_pair->first << "\n" RESET;
				sockets.push_back(SockListen(it_pair->second,
									ip_to_number(it_pair->first.c_str())));
			}
		}
	}
}

void	launch_server(std::vector<Server>& servers,
						std::map<std::string, std::string>& host_ip_lookup)
{
	SockComm*			new_connect;
	std::vector<SockListen> listen_sockets;
	std::vector<SockComm> communication_sockets;
	fd_set				master_socket_list, copy_socket_list;
	char				buffer[BUFFER_SIZE + 1];

	(void)host_ip_lookup;
	// creates the socket
	open_listening_sockets(listen_sockets, servers);
	FD_ZERO(&master_socket_list);
	FD_ZERO(&copy_socket_list);
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
	int	fd_max_nb = listen_sockets.back().get_socket_fd();
	while (true)
	{
		std::cout << MAGENTA "====== Waiting for incoming new connections "
						<< "======\n" RESET;
		/* Copy our original socket list into copy because Select() will erase
		 * its contents and leave only read-ready sockets
		 */
		copy_socket_list = master_socket_list;

		// accept incoming connections
		if (select(fd_max_nb + 1, &copy_socket_list, NULL, NULL, NULL) == -1)
		{
			perror("failed select\n"); 
			throw std::runtime_error("Call to select() failed");
		}
		
		for (int i = 0; i <= fd_max_nb; ++i)
		{
			if (FD_ISSET(i, &copy_socket_list))
			{
				int index = get_socket_index(listen_sockets, i);
				if (index != -1)
				{
					try
					{
						new_connect = listen_sockets[index].\
													accept_connection();
						std::cout << "Before closing any socket\n";
						communication_sockets.push_back(*new_connect);
						FD_SET(new_connect->get_socket_fd(), &master_socket_list);
						if (new_connect->get_socket_fd() > fd_max_nb)
							fd_max_nb = new_connect->get_socket_fd();
						std::cout << 
							GREEN "Server Accepted new connection on socket "
							<< listen_sockets[index].get_port() << "\n"RESET;
					}
					catch (std::exception& e)
					{
						std::cout << e.what() << "\n";
						continue;
					}
				}
				else
				{
					ssize_t nb_bytes = recv(i, buffer, BUFFER_SIZE, 0);
					if (nb_bytes <= 0)
					{
						if (nb_bytes == 0)
							std::cout << "Connection closed from Socket "
										<< i << std::endl;
						else if (nb_bytes < 0)
							perror("Recv failed");
						sock_com_it_t it = communication_sockets.begin()
								+ get_socket_index(communication_sockets, i);
						communication_sockets.erase(it);	
						it->close_socket();
						FD_CLR(i, &master_socket_list);
					}
					else
					{
						std::cout << BLUE "Received data from client " << i
							<< "\n"RESET;
						read_buf(buffer, nb_bytes);
						sock_com_it_t it = communication_sockets.begin()
								+ get_socket_index(communication_sockets, i);
						// if (it->get_server() == NULL)
							// Server* serv = get_server_associated_with_request(
								// servers, *it, buffer);
						request_t *request = get_parsed_request(buffer);
						std::cout << BLUE "Sending data To client " << i
							<< "\n"RESET;
						send_response(request, it->get_socket_fd());
					}
				}
			}
		}
	}
}

int main()
{
	std::vector<Server>					servers;
	std::map<std::string, std::string>	host_ip_lookup;
	if (!parse_config_file("../parser/server_config.conf", servers, 
		host_ip_lookup))
		return 1;
	std::cout << "File is good\n\nStarting WebServer\n";
	launch_server(servers, host_ip_lookup);
	return 0;
}


/* We receive a request:
 * We search IP:PORT pairs
*/
