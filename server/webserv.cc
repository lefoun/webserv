#include "webserver.hpp"


#define BUFFER_SIZE 4096

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

void	read_buf(char buffer[], int size = 0)
{
	if (size == 0)
	{
		for (size_t i = 0; buffer[i] != '\0'; ++i)
			std::cout << buffer[i];
	}
	else
	{
		for (size_t i = 0; i < size; ++i)
			std::cout << buffer[i];
		std::cout << std::endl;
	}
}

void	parse_target_arguments(request_t& request)
{
	size_t pos = request.target.find('?', 0);	
	if (pos == std::string::npos)
		return ;
	request.args = request.target.substr(pos + 1, std::string::npos);
	std::string tmp = request.target.substr(0, pos);
	request.target = tmp;
}

void	check_char_in_stream(const char& delimiter, std::istringstream& ss)
{
	char tmp;

	ss >> tmp;
	if (tmp != delimiter)
		throw std::invalid_argument("Unxpected token");
}

void	parse_request_header(const char buffer[], request_t& request)
{
	/* Request format
	 * request-line = method SP request-target SP HTTP-version CRLF
	 */

	size_t 				i = 0;
	char				tmp_char;
	std::string			tmp;
	std::istringstream	ss(buffer);

	ss >> std::noskipws;
	/* Parse Method */
	ss >> request.method;
	if (!(request.method == "GET" || request.method == "POST"
		|| request.method == "DELETE"))
		throw std::invalid_argument("Invalid Method");
	
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

// bool	check_valid_host(const std::string& host)
// {

// 	std::string::size_type	pos = host.find(':');
// 	if (pos == std::string::npos) /* Means that it's either a Host or a Port */
// 	{
// 		std::cout << "This is either a simple host with default port "
// 					<< host << std::endl;
// 		if (is_number(host))
// 			set_port(host, server);
// 		else
// 			set_ip(host, server);
// 	}
// 	else /* It's a host:port pair */
// 	{
// 		std::cout << "This is a ip:port pair " << host << std::endl;
// 		std::pair<uint16_t, std::string>	ip_port_pair;
// 		std::string	tmp_host = host.substr(0, pos);
// 		std::string	tmp_port = host.substr(pos + 1, std::string::npos);
// 		set_port(tmp_port, server);
// 		set_ip(tmp_host, server);
// 	}
// }

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
		execve("./cgi_test.py", arg, environ);
		std::cout << "Executed process CGI TEST\n";
		exit(0);
	}
	else /* parent */
	{
		wait(NULL);
		std::ifstream response_file("cgi_serv_communication_file.txt");
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

request_t*	get_parsed_request(const char buffer[])
{
	/* check request
	 * HOSTNAME:PORT
	 * PORT
	 * IP
	 * server_name
	*/
	request_t	*request = new request_t;
	parse_request_header(buffer, *request);
	return request;
}

void	send_response(request_t* request, const int& socket_fd,
						const std::string& static_response)
{
	std::string response;
	if (request->method == "GET" && !request->args.empty()
		/*&& request->target.find_last_of(".py") != std::string::npos*/)
	{
		std::cout << GREEN "Calling CGI Python\n" RESET;
		get_cgi_response(request, response);
	}
	else
		response = static_response;
	std::cout << response << std::endl;
	if (send(socket_fd, response.c_str(), response.length(), 0) < 0)
		throw std::runtime_error(
			"Failed to send data to socket " + SSTR(socket_fd));
}

Server*	get_server_associated_with_request(std::vector<Server>& servers,
											const SockComm& socket,
											const char buffer[])
{
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
	for (int i = 0; i < vec.size(); ++i)
	{
		if (vec[i].get_socket_fd() == socket)
			return i;
	}
	return -1;
}

int main()
{
	std::vector<Server>					servers;
	std::map<std::string, std::string>	host_ip_lookup;
	SockComm*							new_connect;
	if (!parse_config_file("../parser/server_config.conf", servers, 
		host_ip_lookup))
		return 1;
	std::cout << "File is good\n\nStarting WebServer\n";

	fd_set				master_socket_list, copy_socket_list;
	const int 			PORT = 42420;
	std::vector<SockListen> listen_sockets;
	std::vector<SockComm> communication_sockets;
	char				buffer[BUFFER_SIZE + 1];
	std::string				serv_response = "HTTP/1.1 200 OK\nContent-Type:"
										" text/html\nContent-Length: ";
	// std::string				follow_up_rsp = 
	// 									"\n\n<html><header>Response form "
	// 									"Serv</header><body><h1>Hello World"
	// 									"</h1></body></html>";
	std::ifstream html_form("form.html");
	std::stringstream tmp_ss; 
	tmp_ss << html_form.rdbuf();
	serv_response.append(SSTR(tmp_ss.str().size()));
	std::string follow_up_rsp = "\n\n" + tmp_ss.str();
	// std::cout << "This is follow_up " << follow_up_rsp << "\nand size "
	// 		<< serv_response.size() <<  std::endl;
	serv_response.append("\nConnection: keep-alive\n");
	serv_response.append(follow_up_rsp);

	html_form.close();
	std::string response_str(serv_response.c_str());

	SockListen socket_liste(PORT, INADDR_ANY);
	listen_sockets.push_back(socket_liste);

	// creates the socket
	FD_ZERO(&master_socket_list);
	FD_ZERO(&copy_socket_list);
	for (std::vector<SockListen>::iterator it = listen_sockets.begin();
		it != listen_sockets.end(); ++it)
	{
		it->bind_socket();
		it->listen_socket();
		FD_SET(it->get_socket_fd(), &master_socket_list);
		std::cout << BLUE "Listening on socket " << it->get_socket_fd() 
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
		
		for (size_t i = 0; i <= fd_max_nb; ++i)
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
						if (it->get_server() == NULL)
							Server* serv = get_server_associated_with_request(
								servers, *it, buffer);
						request_t *request = get_parsed_request(buffer);
						std::cout << BLUE "Sending data To client " << i
							<< "\n"RESET;
						send_response(request, it->get_socket_fd(), serv_response);
						// int sent = send(it->get_socket_fd(), "lolizo", 6, 0);
						// std::cout << "Sent " << sent << " bytes\n";

						// it->close_socket();
						// FD_CLR(i, &master_socket_list);
						// void	serv->process_request(buffer);
					}
				}
			}
		}
	}
	return 0;
}


/* We receive a request:
 * We search IP:PORT pairs
*/
