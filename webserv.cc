#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <stdio.h>
#include <iostream>


/*
 * 1- Create a socket -> call socket()
 * 2- Identify the socket -> bind() a socket to a port number.
 * 3- Listen for incoming connections in the server.
 * 4- Read/Write from/into the socket.
 * 5- Close the socket.
*/

int main()
{
	struct sockaddr_in	address;
	const int PORT = 8081;

	memset(&address, 0, sizeof(address));
	/*
	* IP address for this socket.
	* INADDR_ANY = let the os decide
	* htons converts host to network short and htonl to long.
	*/
	address.sin_family = AF_INET; // Internet protocol
	address.sin_addr.s_addr = htonl(INADDR_ANY); 	address.sin_port = htons(PORT);

	// create the socket
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		perror("Can't create socket");
		return 1;
	}

	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("Bind failed");
		return 2;
	}
	
	if (listen(socket_fd, SOMAXCONN) < 0)
	{
		perror("Listen failed");
		return 3;
	}
	socklen_t addr_size = sizeof(address);
	int new_socket = accept(socket_fd, (struct sockaddr *)& address, &addr_size);
	if (new_socket < 0)
	{
		perror("Accept failed");
		return 4;
	}
	std::cout << "This is socket_fd " << socket_fd << std::endl;
	return 0;
}
