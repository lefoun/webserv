#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <string>
#include <iostream>

#define PORT 8081
#define BUFFER_SIZE 4096

void	read_buf(char buffer[], int size)
{
	for (size_t i = 0; i < size; ++i)
		std::cout << buffer[i];
	std::cout << std::endl;
}

int	error_ret(const std::string &err)
{
	perror(err.c_str());
	exit (1);
	return 1;
}

int main()
{
	fd_set				fd_master, read_fds;
	struct sockaddr_in	socket_addr;
	struct sockaddr_in	client_addr;
	socklen_t			socket_len = sizeof(socket_addr);
	char				buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, BUFFER_SIZE);
	FD_ZERO(&fd_master);
	FD_ZERO(&read_fds);
	int listening_s = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_s < 0)
		return error_ret("Socket()");

	if (fcntl(listening_s, F_SETFL, O_NONBLOCK) == -1)
		return error_ret("Fcntl()");

	memset(&socket_addr.sin_zero, 0, sizeof(socket_addr.sin_zero));
	socket_addr.sin_family = AF_INET;
	int listening_s2 = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_s2 < 0)
		return error_ret("Socket() 2");
	// socket_addr.sin_addr.s_addr = inet_addr("10.19.246.24");
	socket_addr.sin_addr.s_addr = INADDR_ANY;
	socket_addr.sin_port = htons(PORT);

	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(PORT);

	int yes = 1;
	setsockopt(listening_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	setsockopt(listening_s2, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(listening_s, (struct sockaddr *)&socket_addr, socket_len) < 0)
		return error_ret("Bind()");

	// if (bind(listening_s2, (struct sockaddr *)&client_addr, socket_len) < 0)
		// return error_ret("Bind() number 2");
		
	if (listen(listening_s, SOMAXCONN) < 0)
		return error_ret("Listen()");

	if (listen(listening_s2, SOMAXCONN) < 0)
		return error_ret("Listen()2");
	FD_SET(listening_s, &fd_master);
	int new_socket;
	int fdmax = listening_s;
	while (true) // main loop
	{
		read_fds = fd_master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
			return error_ret("Select inside loop");
		
		for (int i = 0; i <= fdmax; ++i)
		{
			if (FD_ISSET(i, &read_fds))
			{
				if (i == listening_s)
				{
					socklen_t client_len = sizeof(client_addr);					
					// new_socket = accept(listening_s, (struct sockaddr *)&client_addr, &client_len);	
					new_socket = accept(listening_s, NULL, NULL);	
					fcntl(new_socket, F_SETFL, O_NONBLOCK);
					if (new_socket == -1)
						perror("Accept() failed to accept new connection");
					else
					{
						FD_SET(new_socket, &fd_master);
						if (new_socket > fdmax)
							fdmax = new_socket;
						std::cout << "Server accepted new connection " << new_socket
							<< std::endl;
					}
				}
				else
				{
					int nbytes = recv(i, buffer, BUFFER_SIZE, 0);
					if (nbytes <= 0)
					{
						if (nbytes == 0)
							std::cout << "Connection closed from Socket " << i << std::endl;
						else if (nbytes < 0)
							perror("Recv failed");
						close(i);
						FD_CLR(i, &fd_master);
					}
					else
					{
						std::cout << "Received data from client " << i << ":\n";
						read_buf(buffer, nbytes);
						std::string serv_response("HTTP/1.1 200 OK\nContent-Type:"
						" text/html\nContent-Length: ");
						std::string header_res(" \n\n<html><header>Response form Serv<header><body> Hello World! </body></html>");
						serv_response.append(std::to_string(header_res.length()));
						serv_response.append(header_res);
						int sent = send(i, serv_response.c_str(), strlen(serv_response.c_str()), 0);
						std::cout << "Sent " << sent << " data\n";
						std::cout << "Received byte " << nbytes << std::endl;
						memset(buffer, 0, BUFFER_SIZE);
					}
				}
			}
		}
	} // end main loop
	return 0;
}
