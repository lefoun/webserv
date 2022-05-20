#pragma once

#include <string>
#include <vector>
#include <utility>
#include <stdint.h>

class Location
{
	public:
		typedef std::pair<uint16_t, std::string>	uint16_str_pair;
	private:
		std::string								_path;
		std::string								_root_path;
		std::string								_index_file;
		std::string								_redirection;
		std::vector<std::string>				_allowed_methods;
		std::vector<uint16_str_pair>			_error_pages;
		bool									_auto_index;
		bool									_is_auto_index_set;
	
	public:
		Location(const std::string& path)
		:
		_path(path),
		_is_auto_index_set(false)
		{}

		std::string&							get_path()
		{ return _path; }
		std::string&							get_root_path()
		{ return _root_path; }
		void									set_root_path(std::string root_path)
		{ _root_path = root_path; }
		std::string&							get_index_file()
		{ return _index_file; }
		void									set_index_file(std::string index_file)
		{ _index_file = index_file; }
		std::string&							get_redirections()
		{return _redirection; }
		std::vector<std::string>&				get_allowed_methods()
		{ return _allowed_methods; }
		std::vector<uint16_str_pair>&			get_error_pages()
		{ return _error_pages; }
		bool&									get_auto_index()
		{return _auto_index;}
		void									set_auto_index(bool auto_index)
		{_auto_index = auto_index; _is_auto_index_set = true;}
		bool&									get_is_auto_index_set()
		{return _is_auto_index_set;}
};
