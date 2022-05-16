#include "webserver.hpp"
#include "colors.hpp"


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

void	parse_request_header(const char buffer[])
{
	/* Request format
	 * request-line   = method SP request-target SP HTTP-version CRLF
	 */
	size_t 				i = 0;
	request_t 			request;
	std::istringstream	ss(buffer);
	std::string			tmp;

	ss >> std::noskipws;
	/* Parse Method */
	ss >> request.method;
	std::cout << "This is method " << request.method << "\n";
	if (!(request.method == "GET" || request.method == "POST"
		|| request.method == "DELETE"))
		throw std::invalid_argument("Invalid Method");
	std::cout << "Parsing Space 1\n";
	char tmp_char;
	ss >> tmp_char;	
	std::cout << "This is tmp |" << tmp_char << "|\n";
	if (tmp_char != ' ')
		throw std::invalid_argument("expected space");
	ss >> request.target;
	ss >> tmp_char;
	std::cout << "Parsing Space 2\n";
	if (tmp_char != ' ')
		throw std::invalid_argument("expected space");
	ss >> request.http_version;
	if (request.http_version != "HTTP/1.1")
		throw std::invalid_argument("expected HTTP/1.1 version");
	ss >> tmp_char;
	std::cout << "this cariage return " << int(tmp_char) << "\n";
	if (tmp_char != '\n' && tmp_char != '\r')
		throw std::invalid_argument("Expected CRLF");
}

Server*	get_server_associated_with_request(std::vector<Server>& servers,
											const SockComm& socket,
											const char buffer[])
{
	parse_request_header(buffer);		
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
	std::vector<Server> servers;
	if (!parse_config_file("../parser/server_config.conf", servers))
		return 1;
	//servers.
	std::cout << "File is good\n\nStarting WebServer\n";

	fd_set				master_socket_list, copy_socket_list;
	const int 			PORT = 42420;
	std::vector<SockListen> listen_sockets;
	SockListen socket_liste(PORT, INADDR_ANY);
	listen_sockets.push_back(socket_liste);
	std::vector<SockComm> communication_sockets;
	char				buffer[BUFFER_SIZE + 1];
	std::string				serv_response = "HTTP/1.1 200 OK\nContent-Type:"
										" text/html\nContent-Length: ";
	std::string				follow_up_rsp = 
										"\n\n<html><header>Response form "
										"Serv</header><body><h1>Hello World"
										"</h1></body></html>";
	serv_response.append(SSTR(serv_response.size()));
	serv_response.append(follow_up_rsp);

	std::string response_str(serv_response.c_str());

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
			throw std::runtime_error("Call to select() failed");
		
		for (size_t i = 0; i <= fd_max_nb; ++i)
		{
			if (FD_ISSET(i, &copy_socket_list))
			{
				int index = get_socket_index(listen_sockets, i);
				if (index != -1)
				{
					try 
					{
						SockComm *new_conect = listen_sockets[index].\
													accept_connection();
						communication_sockets.push_back(*new_conect);
						FD_SET(new_conect->get_socket_fd(), &master_socket_list);
						if (new_conect->get_socket_fd() > fd_max_nb)
							fd_max_nb = new_conect->get_socket_fd();
						std::cout << 
							GREEN "Server Accepted new connection on socket "
							<< listen_sockets[index].get_port() << "\n"RESET;
							if (send(new_conect->get_socket_fd(), 
								serv_response.c_str(),
								serv_response.length(), 0) < 0)
							throw std::runtime_error(
								"Failed to send data to socket " + SSTR(i));
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
						FD_CLR(i, &master_socket_list);
					}
					else if (nb_bytes > BUFFER_SIZE)
						send_response();
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