#pragma once

#include <string>
#include <vector>

class Location
{
	private:
		/* contains [location =/ /some_location] */
		bool						_is_strict_location; 
		std::string					_path;
		std::vector<std::string>	_root_paths;
	
	public:
		Location(std::string& path,
				std::vector<std::string>& root_paths,
				const bool& is_strict = false)
		: _is_strict_location(is_strict), _path(path), _root_paths(root_paths)
		{}

		bool	is_strict_location() { return _is_strict_location; }
		std::string&	get_path() { return _path; }
		std::vector<std::string>&	get_root_paths() { return _root_paths; }
		
};
