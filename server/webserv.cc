#include "webserver.hpp"
#include <vector>

#define BUFFER_SIZE 4096

	/*
	* sin_family = Protocol for this socket. (internet in this case)
	* sin_addr.s_addr = the IP address for this socket
	* INADDR_ANY = let the os decide
	* htons converts host to network short and htonl to long.
	*/

typedef struct sockaddr_in sockaddr_in_t;

int	return_error(const std::string& error_msg)
{
	perror(error_msg.c_str());
	return errno;
}

int main()
{
	Socket mysock(83, 0324);
	parse_config_file("../parser/server_config.conf");
	std::cout << "File is good\n\nStarting WebServer\n";

	fd_set				fd_master, read_fds;
	// std::vector<sockaddr_in_t>	socket_addr;
	std::vector<sockaddr_in_t> socket_addr;
	const int 			PORT = 42420;
	socklen_t 			addr_size = sizeof(address);
	char				*serv_response = "HTTP/1.1 200 OK\nContent-Type:"
										" text/html\nContent-Length: 20"
										"\n\nResponse form Serv";

	std::string response_str(serv_response);

	address.sin_family = AF_INET; // Internet protocol
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	if (address.sin_addr.s_addr == (in_addr_t)(-1))
		return_error("Converting IP address from char* to uint failed");

	// creates the socket
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	int yes = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (socket_fd <= 0)
		return_error("Can't create socket");
	std::cout << "Created the socket " << socket_fd << std::endl;

	// binds the socket to a porn number
	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		return_error("Bind failed");

	// listens to incoming connections
	if (listen(socket_fd, SOMAXCONN) < 0)
		return_error("Listen failed");

	while (true)
	{
		std::cout << "====== Waiting for incoming new connections ======\n";

		// accept incoming connections
		int new_socket = accept(socket_fd, (struct sockaddr *)&address, &addr_size);
		if (new_socket < 0)
			return_error("Accept failed");

		char buffer[BUFFER_SIZE + 1] = {0};
		int val_received = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
		std::cout << "Reading from server: " << std::endl;
		for (size_t i = 0; buffer[i]; ++i)
			std::cout << buffer[i];
		std::cout << std::endl;
		if (val_received == 0)
			std::cout << "Connection closed received 0 bytes\n";
		else if (val_received < 0)
			std::cout << "No bytes to read" << std::endl;

		send(new_socket, response_str.c_str(), response_str.length(), 0);

		std::cout << "====== Closing socket_fd ======" << std::endl;
		close(new_socket);
	}
	return 0;
}
