#include "webserver.hpp"

	/*
	* sin_family = Protocol for this socket. (internet in this case)
	* sin_addr.s_addr = the IP address for this socket
	* INADDR_ANY = let the os decide
	* htons converts host to network short and htonl to long.
	*/
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

bool	is_complete_request(std::string& request, request_t *rqst,
							const std::map<std::string, std::string>&
							host_ip_lookup,
							const char *req_parsing_lookup[REQUEST_KEYS_SIZE])
{
	// read_buf(const_cast<char *>(request.c_str()), request.size());
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


void	launch_server(std::vector<Server>& servers,
						std::map<std::string, std::string>& host_ip_lookup,
						const char *req_parsing_lookup[REQUEST_KEYS_SIZE])
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
											host_ip_lookup, req_parsing_lookup))
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
	static const char					*req_parsing_lookup[REQUEST_KEYS_SIZE];
	if (argc < 2)
		config_file = "server_config.conf";
	else
		config_file = argv[0];
	try
	{
		parse_config_file(config_file, servers, host_ip_lookup);
		init_request_parsing_lookup_tab(req_parsing_lookup);
		std::cout << GREEN "Loaded config file.\n\n"
					<< "Starting WebServer...\n"RESET;
		launch_server(servers, host_ip_lookup, req_parsing_lookup);
		return 0;
	}
	catch (std::exception &e) { std::cout << e.what() << std::endl; }
	return 1;
}
