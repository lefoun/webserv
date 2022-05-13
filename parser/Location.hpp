#pragma once

#include <string>
#include <vector>
#include <utility>
#include <stdint.h>

class Location
{
	public:
		typedef std::pair<std::string, std::vector<uint16_t> >
													str_vec_of_uint16_pair;
	private:
		std::string								_path;
		std::string								_root_path;
		std::string								_index_file;
		std::string								_redirection;
		std::vector<std::string>				_allowed_methods;
		std::vector<str_vec_of_uint16_pair>		_error_pages;
		bool									_auto_index;
		bool									_is_auto_index_set;
	
	public:
		Location(const std::string& path)
		:
		_path(path),
		_error_pages(std::vector<str_vec_of_uint16_pair>(
			1, std::make_pair(std::string(), std::vector<uint16_t>()))),
		_is_auto_index_set(false)
		{}

		std::string&							get_path()
		{ return _path; }
		std::string&							get_root_path()
		{ return _root_path; }
		std::string&							get_index_file()
		{ return _index_file; }
		std::string&							get_redirections()
		{return _redirection; }
		std::vector<std::string>&				get_allowed_methods()
		{ return _allowed_methods; }
		std::vector<str_vec_of_uint16_pair>&	get_error_pages()
		{ return _error_pages; }
		bool&									get_auto_index()
		{return _auto_index;}
		bool&									get_is_auto_index_set()
		{return _is_auto_index_set;}
};
