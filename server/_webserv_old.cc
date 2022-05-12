#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <fcntl.h>
#include <sys/select.h>
#include "webserver.hpp"


#define BUFFER_SIZE 4096

int main()
{
	if (parse_config_file("../parser/server_config.conf"))
		std::cout << "File is good\n\nStarting WebServer\n";
	struct sockaddr_in	address;
	// const int 			PORT = 8081;
	const int 			PORT = 42420;
	socklen_t 			addr_size = sizeof(address);
	char				*serv_response = "HTTP/1.1 200 OK\nContent-Type:"
						" text/plain\nContent-Length: 20\n\nResponse form Serv";
	// std::ifstream		fin("cat_img.jpeg", std::ios::in | std::ios::binary);		
	// std::ifstream		fin("elisa.html", std::ios::in);		

	std::string response_str(serv_response);
	// std::ostringstream	oss;
	// oss << fin.rdbuf();
	// std::string			data(oss.str());
	// std::string			len = std::to_string(data.length());
	// std::string			response_str = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\nContent-Length: ";
	// std::string			response_str = "HTTP/1.1 200 OK\nContent-Type: /html\nContent-Length: ";
	// response_str.append(len);
	// response_str.append("\n\n");
	// response_str.append(data);
	

	/*
	* sin_family = Protocol for this socket. (internet in this case)
	* sin_addr.s_addr = the IP address for this socket
	* INADDR_ANY = let the os decide
	* htons converts host to network short and htonl to long.
	*/
	address.sin_family = AF_INET; // Internet protocol
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	if (address.sin_addr.s_addr == (in_addr_t)(-1))
	{
		perror("Converting IP address from char* to uint failed");
		return 2;
	}

	// creates the socket
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // last argument for the
													// protocol. IPPROTO_TCP
	int yes = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (socket_fd <= 0)
	{
		perror("Can't create socket");
		return 1;
	}
	// fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	std::cout << "Created the socket " << socket_fd << std::endl;
	// binds the socket to a porn number
	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("Bind failed");
		return 2;
	}

	// listens to incoming connections
	if (listen(socket_fd, SOMAXCONN) < 0)
	{
		perror("Listen failed");
		return 3;
	}

	while (true)
	{
		std::cout << "====== Waiting for incoming new connections ======\n";

		// accept incoming connections
		int new_socket = accept(socket_fd, (struct sockaddr *)&address, &addr_size);
		if (new_socket < 0)
		{
			perror("Accept failed");
			return 4;
		}

		char buffer[BUFFER_SIZE + 1] = {0};
		int val_read = read(new_socket, buffer, BUFFER_SIZE - 1);
		// int val_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
		std::cout << "Reading from server: " << std::endl;
		for (size_t i = 0; buffer[i]; ++i)
			std::cout << buffer[i];
		std::cout << std::endl;
		if (val_read == 0)
			std::cout << "Connection closed received 0 bytes\n";
		else if (val_read < 0)
			std::cout << "No bytes to read" << std::endl;

		write(new_socket, response_str.c_str(), response_str.length());

		std::cout << "====== Closing socket_fd ======" << std::endl;
		close(new_socket);
	}
	return 0;
}
