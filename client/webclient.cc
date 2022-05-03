#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 4096

int main()
{
	sockaddr_in	server_addr;
	const int	port = 8081;
	int			socket_fd;
	int			value_read;
	char		*hello_client = "Hello from client";
	char		buffer[BUFFER_SIZE + 1] = {0};

	memset(&server_addr, 0, sizeof(server_addr));
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		perror("Socket() failed");
		return 1;
	}
	std::cout << "Client opened socket " << socket_fd << std::endl;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr("10.19.246.24");

	if (server_addr.sin_addr.s_addr == (in_addr_t)(-1))
	{
		perror("Converting IP address from char* to uint failed");
		return 2;
	}
	
	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Connect failed from client side");
		return 3;
	}
	std::cout << "====== Client connected to server ======" << std::endl;

	int sent_bytes = send(socket_fd, hello_client, strlen(hello_client), 0);

	std::cout << "====== Client sent a message of size " << sent_bytes << ":======" << std::endl;
	value_read = read(socket_fd, buffer, 1024);
	for (size_t i = 0; buffer[i]; ++i)
		std::cout << buffer[i];
	std::cout << std::endl;
	close(socket_fd);
	return 0;
}
