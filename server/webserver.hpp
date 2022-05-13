#pragma once

/* C libraries to enable socket libraries */
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

/* Socket | Inet libraries to enable communication */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

/* C++ Libraries to enable I/O */
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>

/* C++ Libraries to enable container creation and algorithm use */
#include <string>
#include <algorithm>
#include <list>
#include <utility>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

bool	parse_config_file(const std::string& file_name);

class Socket
{
	protected:
		struct sockaddr_in		_socket_addr;
		socklen_t				_sockaddr_len;
		int						_socket_fd;
		uint16_t				_port;
		in_addr_t				_ip;

	public:
		const uint16_t&		get_port() const { return _port; }
		const in_addr_t&	get_ip() const { return _ip; }
		const int&			get_socket_fd() const { return _socket_fd; }
		socklen_t&			get_sockaddr_len() { return _sockaddr_len; }
		sockaddr_in&		get_sockaddr_in() { return _socket_addr; }

};

class SockComm : public Socket
{
	public:
		SockComm(const uint16_t& port, const in_addr_t& ip)
		{
			memset(&_socket_addr, 0, sizeof(_socket_addr));
			_port = port;
			_ip = ip;
		}

		void		set_socket_fd(int socket_fd) { _socket_fd = socket_fd; }
		int			close_socket() { return close(get_socket_fd()); }
		void	init_sock_com()
		{
				if (fcntl(_socket_fd, F_SETFL, O_NONBLOCK) == -1)
					throw std::runtime_error(
						"Fcntl failed for socket " + SSTR(_socket_fd));
				int yes = 1;
				if (setsockopt(
					_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
					throw std::runtime_error(
						"setsockopt failed for socket " + SSTR(_socket_fd));
		}

};

class SockListen : public Socket
{
	private:
	
	/* Need to createa another constructor that includes std::string as
	 * ip_name ?
	 */

	public:
		SockListen(const uint16_t port = 80, const std::string& ip = "0.0.0.0")
		{
			const in_addr_t ip_int = inet_addr(ip.c_str());
			if (ip_int == (in_addr_t)(-1))
				throw std::runtime_error(
					"Failed to convert IP address to in_addr_t");
			_init_socket(port, ip_int);
			std::cout << "Socket " << _socket_fd << "created successfuly\n";
		}
		SockListen(const uint16_t port = 80, const in_addr_t ip = INADDR_ANY)
		{
			_init_socket(port, ip);
		}

		~SockListen()
		{
			std::cout << "CLOSED SOCKET " << _socket_fd << '\n';
			close(_socket_fd);
		}

	/* Class Getters : Return const because we don't need to modify the values*/
		SockComm&			accept_connection()
		{
			SockComm	*socket_comm = new SockComm(this->get_port(), this->get_ip());
			int new_socket = accept(this->get_socket_fd(), 
									(struct sockaddr*)&socket_comm->\
									get_sockaddr_in(),
									&socket_comm->get_sockaddr_len());
			if (new_socket < 0)
				throw std::runtime_error(
					"Couldn't accept new connection from socket " 
					+ SSTR(get_socket_fd()));
			socket_comm->set_socket_fd(new_socket);
			socket_comm->init_sock_com();
			return *socket_comm;
		}

	/* Members to handle binding and listneing */
		int				bind_socket()
		{
			if (bind(get_socket_fd(),
					(struct sockaddr *)&_socket_addr, get_sockaddr_len()) < 0)
				throw std::runtime_error(
					"Socket " + SSTR(get_socket_fd()) + "Failed to open");
			return 0;
		}
		void			listen_socket()
		{
			if (listen(get_socket_fd(), SOMAXCONN) < 0)
				throw std::runtime_error(
					"Socket " + SSTR(get_socket_fd()) + "Failed to listen");

		}
	
		int	close_socket() { return close(get_socket_fd()); }

	private:
		void	_init_socket(const uint16_t& port, const in_addr_t& ip)
		{
			_port = port;
			_ip = ip;
			/* Init sockaddr_in */
			memset(&_socket_addr.sin_zero, 0, sizeof(_socket_addr.sin_zero));
			_socket_addr.sin_family = AF_INET;
			_socket_addr.sin_addr.s_addr = ip;
			_socket_addr.sin_port = htons(_port);

			/* Open socket */
			_sockaddr_len = sizeof(_socket_addr);
			_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (_socket_fd < 0)
				throw std::runtime_error(
					"Socket of port " + SSTR(_port) + "and Ip " + SSTR(_ip));
			if (fcntl(_socket_fd, F_SETFL, O_NONBLOCK) == -1)
				throw std::runtime_error(
					"Fcntl failed for socket " + SSTR(_socket_fd));
			int yes = 1;
			if (setsockopt(
				_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
				throw std::runtime_error(
					"setsockopt failed for socket " + SSTR(_socket_fd));
			/* Tmp print to debug */
		}
};

/* PARSING *
 * Parsing lexical
 * Parsing sytaxique
 * Créer nos objets servers
 * Trier les membres (ports, ips, paires ips-ports) de nos objets
 * Créer les sockets et les binds à des ports. (Si un port n'a pas d'ip assignée)
 * - alors ouvrir le port et le bind a ADDR_ANY sinon si le port a une ip assignée
 * - mais a aussi un listen sans aucune ip assignée alors ouvrir le port et le bind
 * - a ADDR_ANY sinon si il n y a qu'une adresse ip assignée à une ip alors 
 * - ouvrir le port et le bind à l'adresse assignée sinon si qu'une adresse
 * - ip est spécifiée alors la bind au port 80.
 * exemples des trois cas cités ci-dessus:
 *  1- Listen 80;
 *  2- Listen localhost:9191; listen 9191;
 *  3- listen bidule:8888;
 *  4- listen localhost;
 */

/* Règles d'ouverture de port
 * si il n y a que des pairs il n'ouvre que des pairs
 * si il y a des ip seules il ouvre ip:80
 * 
 ****************************** NOUS ****************************
 * prendre les ports les ouvrir (check si le port est ouvert ouppa)
 * 
*/
/*
 * CREATE SERVER
 * créer une socket par 
 * créer une sock_addr_in
 * 

*/

/*
 * READ FROM SERVER *
 * on reçoit une requête :
 * case 1: Les 

*/

// class Socket_addr_in
// {
// 	private:
// 		uint8_t			_sin_len;
// 		sa_family_t		_sin_family;
// 		in_port_t		_sin_port;
// 		struct in_addr	_sin_addr;
// 		char			_sin_zero[8];
	
// 	public:
// 		Socket_addr_in(const struct sockaddr_in& sockaddr)
// 		:
// 		_sin_addr(sockaddr.sin_addr),
// 		_sin_family(sockaddr.sin_family),
// 		_sin_port(sockaddr.sin_port)
// 		{
// 			for (size_t i = 0; i < 8; ++i)
// 				_sin_zero[i] = sockaddr.sin_zero[i];
// 		}
// 		Socket_addr_in() {}

// };
