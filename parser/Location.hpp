#pragma once

#include <string>
#include <vector>

class Location
{
	private:
		/* contains [location =/ /some_location] */
		const bool						_is_strict_location; 
		const std::string				_path;
		const std::vector<std::string>	_root_paths;
	
	public:
		Location(const std::string& path,
				const std::vector<std::string>& root_paths,
				const bool& is_strict = false)
		: _is_strict_location(is_strict), _path(path), _root_paths(root_paths)
		{}

		const bool	is_strict_location() const { return _is_strict_location; }
		const std::string&	get_path() const { return _path; }
		const std::vector<std::string>&	get_root_paths() { return _root_paths; }
		
};
