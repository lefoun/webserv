#pragma once

#include "webserver.hpp"

/* 2 typedefs to make code more readable by reducing the length of it */
typedef struct sockaddr_in				sockaddr_in_t;
typedef std::vector<SockComm>::iterator	sock_com_it_t;

void	close_socket(const ssize_t nb_bytes, fd_set& master_socket_list,
						sock_com_it_t& socket_it, std::vector<SockComm>&
						communication_sockets);


void	open_listening_sockets(std::vector<SockListen>& sockets,
								std::vector<Server>& servers);

void	bind_sockets(std::vector<SockListen>& listen_sockets,
						fd_set& master_socket_list);

template <typename T>
int get_socket_index(const std::vector<T>& vec, const int& socket)
{
	for (size_t i = 0; i < vec.size(); ++i)
	{
		if (vec[i].get_socket_fd() == socket)
			return i;
	}
	return -1;
}

template <typename U>
bool	is_socket_already_open(const U vect, uint16_t port, in_addr_t ip)
{
	for (size_t i = 0; i < vect.size(); ++i)
	{
		if (vect[i].get_ip() == 0 && vect[i].get_port() == port)
			return true;
		else if (port == vect[i].get_port() && ip == vect[i].get_ip())
			return true;
	}
	return false;
}
