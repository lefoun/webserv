#pragma once

#include "webserver.hpp"

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
	private:
		Server*		_server;
		std::string	_client_request;
		request_t	_parsed_request;
		response_t	_response;

	public:

		SockComm(const uint16_t& port, const in_addr_t& ip, Server* server = NULL)
		{
			memset(&_socket_addr, 0, sizeof(_socket_addr));
			_port = port;
			_ip = ip;
			_server = server;
			_parsed_request.body_parsing_state = NOT_STARTED;
			_parsed_request.is_content_length_set = false;
		}

		SockComm(const SockComm& copy)
		{
			*this = copy;
			std::cout << RED "Callign copy constructor\n" RESET;
			_parsed_request.body_parsing_state = NOT_STARTED;
			_parsed_request.is_content_length_set = false;
		}

		SockComm&	operator=(const SockComm& cop)
		{
			std::cout << RED "Calling assignment operator\n" RESET;
			this->_port = cop.get_port();
			this->_ip = cop.get_ip();
			this->_socket_fd = cop.get_socket_fd();
			this->_sockaddr_len = cop._sockaddr_len;
			this->_socket_addr = cop._socket_addr;
			/* need a profound copy here */
			_parsed_request = cop._parsed_request;
			return *this;
		}

		~SockComm()
		{
			std::cout << RED "Calling Sock Com destructor\n" RESET<< std::endl;
			// std::cout << "closing socket comm " << get_socket_fd() << std::endl;
			// close(_socket_fd);
		}

		Server*			get_server() { return _server; }
		std::string&	get_client_request() { return _client_request; }
		void			set_socket_fd(int socket_fd) { _socket_fd = socket_fd; }
		int				close_socket() { return close(get_socket_fd()); }
		request_t&		get_request() { return _parsed_request; }
		response_t&		get_response() { return _response; }
		void			init_sock_com()
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
	public:
		SockListen(const uint16_t port = 80, const std::string& ip = "0.0.0.0") : Socket()
		{
			const in_addr_t ip_int = inet_addr(ip.c_str());
			if (ip_int == (in_addr_t)(-1))
				throw std::runtime_error(
					"Failed to convert IP address to in_addr_t");
			_init_socket(port, ip_int);
			std::cout << "Socket " << _socket_fd << "created successfuly\n";
		}
		SockListen(const uint16_t port = 80, const in_addr_t ip = INADDR_ANY) : Socket()
		{
			_init_socket(port, ip);
		}

		SockListen(const SockListen& copy)
		{
			// std::cout << RED "Calling copy operator of SockListen\n" RESET;
			*this = copy;
		}

		SockListen& operator=(const SockListen& copy)
		{
			// std::cout << RED "Calling assignment operator of SockListen\n" RESET;
			this->_port = copy.get_port();
			this->_ip = copy.get_ip();
			this->_socket_fd = copy.get_socket_fd();
			this->_sockaddr_len = copy._sockaddr_len;
			this->_socket_addr = copy._socket_addr;
			return *this;
		}
		~SockListen()
		{
			// std::cout << "CLOSED SOCKET " << _socket_fd << '\n';
			// close(_socket_fd);
		}

	/* Class Getters : Return const because we don't need to modify the values*/
		SockComm*			accept_connection()
		{
			SockComm	*socket_comm = new SockComm(_port, _ip);
			int new_socket = accept(_socket_fd,
									(struct sockaddr*)&(socket_comm->\
									get_sockaddr_in()),
									&socket_comm->get_sockaddr_len());
			if (new_socket < 0)
				throw std::runtime_error(
					"Couldn't accept new connection from socket "
					+ SSTR(get_socket_fd()));
			socket_comm->set_socket_fd(new_socket);
			socket_comm->init_sock_com();
			return socket_comm;
		}

	/* Members to handle binding and listneing */
		void			bind_socket()
		{
			if (bind(get_socket_fd(),
					(const struct sockaddr *)&_socket_addr, get_sockaddr_len()) < 0)
				throw std::runtime_error(
					"Socket " + SSTR(get_socket_fd()) + "Failed to bind");
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
			_socket_addr.sin_addr.s_addr = htonl(ip);
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
