#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>


/*
 * 1- Create a socket.
 * 2- Identify the socket.
 * 3- Listen for incoming connections in the server.
 * 4- Read/Write from/into the socket.
 * 5- Close the socket.
*/

int main()
{
	struct sockaddr	address;
	const int PORT = 8080;

	memset(&address, 0, sizeof(address));
	address.sa_family = AF_INET;
	address.
	int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		perror("Can't create socket");
		return 1;
	}
	std::cout << "This is socket_fd " << socket_fd << std::endl;
	return 0;
}
