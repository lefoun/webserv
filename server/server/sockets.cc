#include "../includes/webserver.hpp"

void	close_socket(const ssize_t nb_bytes, fd_set& master_socket_list,
						sock_com_it_t& socket_it, std::vector<SockComm>&
						communication_sockets)
{
	if (nb_bytes == 0)
		std::cout << RED"Connection closed from Socket "
					<< socket_it->get_socket_fd() << RESET << std::endl;
	else if (nb_bytes < 0)
		perror("Recv failed");
	FD_CLR(socket_it->get_socket_fd(), &master_socket_list);
	communication_sockets.erase(socket_it);
	socket_it->close_socket();
}


void	open_listening_sockets(std::vector<SockListen>& sockets,
								std::vector<Server>& servers)
{
	/* find listening ports */
	for (std::vector<Server>::iterator it = servers.begin();
			it != servers.end(); ++it)
	{
		for (std::vector<uint16_t>::iterator it_port =
				it->get_listening_ports().begin();
				it_port != it->get_listening_ports().end(); ++it_port)
		{
			if (!is_socket_already_open(sockets, *it_port, INADDR_ANY))
			{
				std::cout << "Opening Socket PORT: " << *it_port
							<< " IP: ALL\n";
				sockets.push_back(SockListen(*it_port, INADDR_ANY));
			}
		}
	}

	/* find listening ips with default port = 80*/
	for (std::vector<Server>::iterator it = servers.begin();
			it != servers.end(); ++it)
	{
		for (std::vector<std::string>::iterator it_ip =
				it->get_listening_ips().begin();
				it_ip!= it->get_listening_ips().end(); ++it_ip)
		{
			if (!is_socket_already_open(sockets, 8000,
				ip_to_number(it_ip->c_str())))
			{
				std::cout << "Opening Socket PORT: 8000"
							<< " IP: " << *it_ip << "\n";
				sockets.push_back(
					SockListen(8000, ip_to_number(it_ip->c_str())));
			}
		}
	}

	/* find listening pairs Ip:port */
	for (std::vector<Server>::iterator it = servers.begin();
			it != servers.end(); ++it)
	{
		for (std::vector<Server::ip_port_pair>::iterator it_pair =
				it->get_ip_port_pairs().begin();
				it_pair!= it->get_ip_port_pairs().end(); ++it_pair)
		{
			if (it_pair->second != 0
				&& !is_socket_already_open(sockets, it_pair->second,
				ip_to_number(it_pair->first.c_str())))
			{
				std::cout << "Opening Socket pair PORT: "
							<< it_pair->second
							<< " IP: " << it_pair->first << "\n";
				sockets.push_back(SockListen(it_pair->second,
									ip_to_number(it_pair->first.c_str())));
			}
		}
	}
}

void	bind_sockets(std::vector<SockListen>& listen_sockets,
						fd_set& master_socket_list)
{
	for (std::vector<SockListen>::iterator it = listen_sockets.begin();
		it != listen_sockets.end(); ++it)
	{
		it->bind_socket();
		it->listen_socket();
		FD_SET(it->get_socket_fd(), &master_socket_list);
		std::cout << BLUE "Listening on socket " << it->get_socket_fd()
				<< " and port "<< it->get_port()
				<< " and ip " << inet_ntoa(it->get_sockaddr_in().sin_addr)
				<< "\n" RESET;
	}
}
