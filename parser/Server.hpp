#pragma once

#include <string>
#include <vector>
#include "Location.hpp"

class Server
{
	private:
		typedef std::pair<uint16_t, std::string>	ip_port_pair; 
		typedef std::pair<std::string, std::string>	str_str_pair;

		std::vector<std::string>				_server_names;
		std::vector<uint16_t>					_listening_ports;
		std::vector<std::string>				_listening_ips;
		std::vector<ip_port_pair>				_listening_port_ip_pairs;
		std::vector<Location>					_locations;
		std::string								_index_file;
		std::vector<std::string>				_error_files;
		std::vector<str_str_pair>				_redirections;
		std::vector<std::string>				_allowd_methods;
		std::string								_root_path;
		bool									_auto_index;
		bool									_is_auto_index_set;

	public:
		std::vector<std::string>&			get_server_names()
		{ return _server_names; }
		std::vector<uint16_t>&				get_listening_ports()
		{ return _listening_ports; }
		std::vector<std::string>&			get_listening_ips()
		{ return _listening_ips; }
		std::vector<ip_port_pair>&			get_ip_port_pairs()
		{ return _listening_port_ip_pairs; }
		std::vector<Location>&				get_locations()
		{ return _locations; }
		std::string&						get_index_file()
		{ return _index_file; }
		std::vector<std::string>&			get_error_files()
		{ return _error_files; }
		std::vector<str_str_pair>&			get_redirections()
		{ return _redirections; }
		std::vector<std::string>&			get_allowd_methods()
		{ return _allowd_methods; }
		std::string&						get_root_path()
		{ return _root_path; }
		bool&								get_auto_index()
		{return _auto_index;}
		bool&								get_is_auto_index_set()
		{return _is_auto_index_set;}
};
