#include "webserver.hpp"
#include <vector>
#include "colors.hpp"


#define BUFFER_SIZE 4096

	/*
	* sin_family = Protocol for this socket. (internet in this case)
	* sin_addr.s_addr = the IP address for this socket
	* INADDR_ANY = let the os decide
	* htons converts host to network short and htonl to long.
	*/

void	read_buf(char buffer[], int size)
{
	for (size_t i = 0; i < size; ++i)
		std::cout << buffer[i];
	std::cout << std::endl;
}

typedef struct sockaddr_in sockaddr_in_t;

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
//	parse_config_file("../parser/server_config.conf");
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
		std::cout << "====== Waiting for incoming new connections ======\n";
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
					SockComm *new_conect = listen_sockets[index].accept_connection();
					FD_SET(new_conect->get_socket_fd(), &master_socket_list);
					if (new_conect->get_socket_fd() > fd_max_nb)
						fd_max_nb = new_conect->get_socket_fd();
					std::cout << 
						GREEN "Server Accepted new connection on socket "
						<< listen_sockets[index].get_port() << "\n"RESET;
						send(new_conect->get_socket_fd(), 
							serv_response.c_str(),
							strlen(serv_response.c_str()), 0);
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
						close(i);
						FD_CLR(i, &master_socket_list);
					}
					else
					{
						std::cout << BLUE "Received data from client " << i
							<< "\n"RESET;
						read_buf(buffer, nb_bytes);
					}
				}
			}
		}
	}
	//	int new_socket = accept(, (struct sockaddr *)&address, &addr_size);
	//	if (new_socket < 0)
	//		return_error("Accept failed");

//		char buffer[BUFFER_SIZE + 1] = {0};
//		int val_received = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
//		std::cout << "Reading from server: " << std::endl;
//		for (size_t i = 0; buffer[i]; ++i)
//			std::cout << buffer[i];
//		std::cout << std::endl;
//		if (val_received == 0)
//			std::cout << "Connection closed received 0 bytes\n";
//		else if (val_received < 0)
//			std::cout << "No bytes to read" << std::endl;
//
//		send(new_socket, response_str.c_str(), response_str.length(), 0);
//
//		std::cout << "====== Closing socket_fd ======" << std::endl;
//		close(new_socket);
//	}
	return 0;
}
