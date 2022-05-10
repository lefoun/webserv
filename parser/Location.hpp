#pragma once

#include <string>
#include <vector>

class Location
{
	private:
		/* contains [location =/ /some_location] */
		typedef std::pair<std::string, std::string>	str_str_pair;

		std::string					_path;
		std::string					_root_path;
		std::string					_index_file;
		std::vector<str_str_pair>	_redirections;
		std::vector<std::string>	_allowed_requests;
	
	public:
		Location(const std::string& path, const std::string& root_path, 
				const std::string& index)
		: _path(path), _root_path(root_path), _index_file(index)
		{}

		std::string&				get_path()
		{ return _path; }
		std::string&				get_root_path()
		{ return _root_path; }
		std::string&				get_index_file()
		{ return _index_file; }
		std::vector<str_str_pair>&	get_redirections()
		{return _redirections; }
		std::vector<std::string>&	get_allowed_requestes()
		{ return _allowed_requests; }
};
