#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 4096

int main()
{
	sockaddr_in	server_addr;
	int			socket_fd;
	int			value_read;
	char		*hello_client = "Hello from client";
	char		buffer[BUFFER_SIZE + 1] = {0};

	memset(&server_addr, 0, sizeof(server_addr));
	socket_fd = socket(AF_INET, SOCK_STREAM, SOMAXCONN);
	if (socket_fd < 0)
	{
		perror("Socket() failed");
		return 1;
	}
	std::cout << "Client opened socket " << socket_fd << std::endl;

}
