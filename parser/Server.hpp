#pragma once

#include <string>
#include <vector>
#include <utility>
#include "Location.hpp"

class Server
{
	public:
		typedef std::pair<uint16_t, std::string>	ip_port_pair; 
		typedef std::pair<std::string, std::vector<uint16_t> >
													str_vec_of_uint16_pair;
	private:

		std::vector<std::string>				_server_names;
		std::vector<uint16_t>					_listening_ports;
		std::vector<std::string>				_listening_ips;
		std::vector<ip_port_pair>				_listening_port_ip_pairs;
		std::vector<Location>					_locations;
		std::string								_index_file;
		std::vector<str_vec_of_uint16_pair>		_error_pages;
		std::vector<std::string>				_allowd_methods;
		std::string								_root_path;
		std::string								_redirection;
		uint8_t									_client_max_body_size;
		bool									_auto_index;
		bool									_is_auto_index_set;
		bool									_is_client_max_body_size_set;

	public:
		Server()
		:
		_server_names(),
		_listening_ports(),
		_listening_port_ip_pairs(1, std::make_pair(0, "0")),
		_error_pages(std::vector<str_vec_of_uint16_pair>(
			1, std::make_pair(std::string(), std::vector<uint16_t>()))),
		_client_max_body_size(1),
		_is_auto_index_set(false),
		_is_client_max_body_size_set(false)
		{}

		std::vector<std::string>&				get_server_names()
		{ return _server_names; }
		std::vector<uint16_t>&					get_listening_ports()
		{ return _listening_ports; }
		std::vector<std::string>&				get_listening_ips()
		{ return _listening_ips; }
		std::vector<ip_port_pair>&				get_ip_port_pairs()
		{ return _listening_port_ip_pairs; }
		std::vector<Location>&					get_locations()
		{ return _locations; }
		std::string&							get_index_file()
		{ return _index_file; }
		std::vector<str_vec_of_uint16_pair>&	get_error_pages()
		{ return _error_pages; }
		std::vector<std::string>&				get_allowed_methods()
		{ return _allowd_methods; }
		std::string&							get_root_path()
		{ return _root_path; }
		std::string&            				get_redirections()
		{ return _redirection; }
		uint8_t&								get_client_max_body_size()
		{ return _client_max_body_size; }
		bool&									get_auto_index()
		{ return _auto_index; }
		bool&									get_is_auto_index_set()
		{ return _is_auto_index_set; }
		bool&									get_is_client_body_size_set()
		{ return _is_client_max_body_size_set; }
};
