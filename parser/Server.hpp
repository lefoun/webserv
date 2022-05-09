#pragma once

#include <string>
#include <vector>
#include "Location.hpp"

class Server
{
	private:
		typedef std::pair<uint16_t, std::string>	ip_port_pair; 

		std::vector<std::string>				_server_names;
		std::vector<uint16_t>					_listening_ports;
		std::vector<std::string>				_listening_ips;
		std::vector<ip_port_pair>				_listening_port_ip_pairs;
		std::vector<Location>					_locations;
		std::vector<std::string>				_index_files;
		std::vector<std::string>				_error_files;
		std::string								_root;

	public:
		std::vector<std::string>&			get_server_names()
		{ return _server_names; }
		std::vector<uint16_t>&			get_listening_ports()
		{ return _listening_ports; }
		std::vector<std::string>&			get_listening_ips()
		{ return _listening_ips; }
		std::vector<ip_port_pair>&		get_ip_port_pairs()
		{ return _listening_port_ip_pairs; }
		std::vector<Location>&			get_locations()
		{ return _locations; }
		std::vector<std::string>&			get_index_files()
		{ return _index_files; }
		std::vector<std::string>&			get_error_files()
		{ return _error_files; }
};
