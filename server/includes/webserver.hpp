#pragma once

/* C libraries to enable socket libraries */
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <ctime>

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
#include <ios>

/* C++ Libraries to enable container creation and algorithm use */
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <utility>

#include "helper_functions.hpp"
#include "Server.hpp"
#include "request_parsing.hpp"
#include "colors.hpp"
#include "response.hpp"

/* for the buffer that reads the clients' message */
// #define BUFFER_SIZE 1048576
#define BUFFER_SIZE 4096 
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF "\r\n"

class Socket;
class SockComm;

/* 2 typedefs to make code more readable by reducing the length of it */
typedef struct sockaddr_in				sockaddr_in_t;
typedef std::vector<SockComm>::iterator	sock_com_it_t;

bool	parse_config_file(const std::string& file_name,
							std::vector<Server>& servers,
							std::map<std::string, std::string>& host_ip_lookup);



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
		static size_t	session; 

		SockComm(const uint16_t& port, const in_addr_t& ip, Server* server = NULL)
		{
			++session;
			memset(&_socket_addr, 0, sizeof(_socket_addr));
			_port = port;
			_ip = ip;
			_server = server;
			_parsed_request.body_parsing_state = NOT_STARTED;
			// _parsed_request = new request_t;
		}

		SockComm(const SockComm& copy)
		{
			++session;
			*this = copy;
			std::cout << RED "Callign copy constructor\n" RESET;
			_parsed_request.body_parsing_state = NOT_STARTED;
			// _parsed_request = new request_t;
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

void	initialize_html_return_code_page(t_return_codes *return_codes);

void		set_response(Server & server, request_t* request,
									response_t* response);
Location	*choose_location(Server & server, request_t* request);
void		change_default_html_return_code(std::string path,
									std::string *return_code);
void	set_location_options(Server & server, request_t* request,
									Location & location, response_t* response);
void	choose_return_code_for_requested_ressource(Server& server,
									request_t* request, response_t* response);

template <typename T>
void	set_default_return_code(T & datas)
{
	std::vector<uint16_str_pair>::iterator it;
	std::vector<uint16_str_pair>::iterator it_end;
	std::string root_path;
	t_return_codes *return_codes;

	it = datas.get_error_pages().begin();
	it_end = datas.get_error_pages().end();
	return_codes = &datas.return_codes;
	initialize_html_return_code_page(&(*return_codes));
	root_path = datas.get_root_path();
	if (--(*root_path.end()) != '/')
		root_path += '/';
	for (; it != it_end; ++it)
	{
		switch (it->first)
		{
			case 400:
				change_default_html_return_code(root_path + it->second, &return_codes->err_400);
				break;
			case 401:
				change_default_html_return_code(root_path + it->second, &return_codes->err_401);
				break;
			case 403:
				change_default_html_return_code(root_path + it->second, &return_codes->err_403);
				break;
			case 404:
				change_default_html_return_code(root_path + it->second, &return_codes->err_404);
				break;
			case 405:
				change_default_html_return_code(root_path + it->second, &return_codes->err_405);
				break;
			case 406:
				change_default_html_return_code(root_path + it->second, &return_codes->err_406);
				break;
			case 408:
				change_default_html_return_code(root_path + it->second, &return_codes->err_408);
				break;
			case 413:
				change_default_html_return_code(root_path + it->second, &return_codes->err_413);
				break;
			case 414:
				change_default_html_return_code(root_path + it->second, &return_codes->err_414);
				break;
			case 429:
				change_default_html_return_code(root_path + it->second, &return_codes->err_429);
				break;
			case 431:
				change_default_html_return_code(root_path + it->second, &return_codes->err_431);
				break;
			case 500:
				change_default_html_return_code(root_path + it->second, &return_codes->err_500);
				break;
			case 501:
				change_default_html_return_code(root_path + it->second, &return_codes->err_501);
				break;
			case 502:
				change_default_html_return_code(root_path + it->second, &return_codes->err_502);
				break;
			case 503:
				change_default_html_return_code(root_path + it->second, &return_codes->err_503);
				break;
			case 504:
				change_default_html_return_code(root_path + it->second, &return_codes->err_504);
				break;
			case 505:
				change_default_html_return_code(root_path + it->second, &return_codes->err_505);
				break;
			case 511:
				change_default_html_return_code(root_path + it->second, &return_codes->err_511);
				break;
			default:
				break;
		}
	}
}
