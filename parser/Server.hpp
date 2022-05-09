#pragma once

#include <string>
#include <vector>
#include "Location.hpp"

class Server
{
	private:
		typedef std::pair<uint16_t, std::string>	ip_port_pair; 

		const std::vector<std::string>				_server_names;
		const std::vector<uint16_t>					_listening_ports;
		const std::vector<std::string>				_listening_ips;
		const std::vector<ip_port_pair>				_listening_port_ip_pairs;
		const std::vector<Location>					_locations;
		const std::vector<std::string>				_index_files;
		const std::vector<std::string>				_error_files;
		const std::string							_root;

	public:
		const std::vector<std::string>&			get_server_names() const
		{ return _server_names; }
		const std::vector<uint16_t>&			get_listening_ports() const
		{ return _listening_ports; }
		const std::vector<std::string>&			get_listening_ips() const
		{ return _listening_ips; }
		const std::vector<ip_port_pair>&		get_ip_port_pairs() const
		{ return _listening_port_ip_pairs; }
		const std::vector<Location>&			get_locations() const
		{ return _locations; }
		const std::vector<std::string>&			get_index_files() const
		{ return _index_files; }
		const std::vector<std::string>&			get_error_files() const
		{ return _error_files; }
		const std::string&						get_root() const
		{ return _root; }
};
