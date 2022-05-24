#include <unistd.h>
#include <dirent.h>
#include "webserver.hpp"
#include "response.hpp"
#include "helper_functions.hpp"
#define DEFAULT_ROOT_PATH "../www"


//Changer ca pour le bon return code
//Comment fait on pour transmettre le file ? 
//Comment fait on pour differencier deux 200 ?? autoindex ok ou file a renvoyer ok
//Il faut check si les methodes sont autorisees quand il n'y a pas 
//de location (le serveur sans options)

void	fill_response(response_t* response,
						const bool& return_code = 200,
						const uint8_t& state = NOT_STARTED,
						const bool& is_auto_index = false,
						const std::string& location = "/",
						const std::string& file_path = "/",
						const std::string& body = "",
						const std::string& content_type = "text/html")
{
	response->return_code = return_code;
	response->is_auto_index = is_auto_index;
	response->date = get_current_time();
	response->content_type = content_type;
	response->location = location;
	response->body = body;
	response->response_state = state;
	response->file_path = file_path;

}

void	choose_return_code_for_requested_ressource(Server& server, request_t* request, response_t* response)
{
	if (request->method.compare("GET") == 0)
	{
		if (!server.get_allowed_methods().empty())
		{
			std::vector<std::string>::iterator it = server.get_allowed_methods().begin();
			for (; it != server.get_allowed_methods().end(); ++it)
			{
				std::cout << "allowed method : " << *it << std::endl;
				if (request->method.compare(*it) == 0)
					break;
			}
			if  (it == server.get_allowed_methods().end())
			{
				//PENSER A AJOUTER UN THROW QUAND E PROJET SERA PLUS PROPRE
				std::cout << server.return_codes.err_405 << std::endl;
				return fill_response(response, 405, COMPLETE, false, "", "", server.return_codes.err_405);
			}
		}
		std::string full_path = server.get_root_path() + request->target;
		DIR *dir = opendir(full_path.c_str());
		if (dir != NULL)
		{
			closedir(dir);
			if (*(--full_path.end()) != '/')
				return fill_response(response, 301, COMPLETE, false, request->target.append("/"));
			else
			{
				full_path.append(server.get_index_file());
				if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) == 0)
					return fill_response(response, 200, NOT_STARTED, false, full_path);
				else if (server.get_auto_index() == true)
					return fill_response(response, 200, NOT_STARTED, true, full_path);
				else
					return fill_response(response, 403, COMPLETE, false, "", server.return_codes.err_403);
			}
		}
		else if (access(full_path.c_str(), R_OK) == 0)
			return fill_response(response, 200, NOT_STARTED, false, full_path);
		else if (access(full_path.c_str(), R_OK) != 0)
			return fill_response(response, 403, COMPLETE, false, "", server.return_codes.err_403);
		else
			return fill_response(response, 404, COMPLETE, false, "", server.return_codes.err_404);
	}
	fill_response(response, 500, COMPLETE, false, "", server.return_codes.err_500);
}

void	set_location_options(Server & server, request_t* request, Location & location, response_t* response)
{
	(void)response;
	std::string root_path = server.get_root_path();
	std::string index_file = server.get_index_file();
	bool autoindex = server.get_auto_index();

	if (request->method.compare("GET") == 0)
	{
		std::cout << location.get_path() << std::endl;
		std::cout << "GET request->inside set_location_options" << std::endl;
		if (!location.get_allowed_methods().empty())
		{
			std::vector<std::string>::iterator it = location.get_allowed_methods().begin();
			for (; it != location.get_allowed_methods().end(); ++it)
			{
				std::cout << "allowed method : " << *it << std::endl;
				if (request->method.compare(*it) == 0)
					break;
			}
			if  (it == location.get_allowed_methods().end())
			{
				//PENSER A AJOUTER UN THROW QUAND E PROJET SERA PLUS PROPRE
				std::cout << location.return_codes.err_405 << std::endl;
				return fill_response(response, 405, COMPLETE, false, "", server.return_codes.err_405);
			}
		}
		if (!location.get_redirections().empty())
		{
			std::string new_url = location.get_redirections() + request->target;
			std::cout << "301 REDIRECTION (new URL = " << new_url << " )" << std::endl;
			return fill_response(response, 301, COMPLETE, false, new_url);
		}
		std::string full_path = root_path + request->target;
		DIR *dir = opendir(full_path.c_str());
		if (dir != NULL)
		{
			closedir(dir);
			if (*(--full_path.end()) != '/')
				return fill_response(response, 301, COMPLETE, false, request->target.append("/"));
			else
			{
				full_path.append(index_file);
				if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) == 0)
					return fill_response(response, 200, NOT_STARTED, false, "", full_path); 
				else if (autoindex == true)
					return fill_response(response, 200, NOT_STARTED, true, "", full_path); 
				else
					return fill_response(response, 403, COMPLETE, false, "", "", server.return_codes.err_403); 
			}
		}
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) == 0)
			return fill_response(response, 200, NOT_STARTED, false, "", full_path); 
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) != 0)
				return fill_response(response, 403, COMPLETE, false, "", "", server.return_codes.err_403); 
		else
			return fill_response(response, 404, COMPLETE, false, "", "", server.return_codes.err_404); 
	}
	return fill_response(response, 500, COMPLETE, false, "", "", server.return_codes.err_500); 
}

// Check if the required URI correspond to a directory and if the URI finish with a slash. If no, we directly redirect to the directory without the slash.
Location *choose_location(Server & server, request_t* request, response_t *response)
{
	(void)response;
	Location				*location = NULL;
	std::string target =	request->target;
	std::vector<Location>::iterator	it;
	int						max_len = -1;

	for (it = server.get_locations().begin(); it != server.get_locations().end(); ++it)
	{
		if (target.compare(0, it->get_path().size(), it->get_path()) == 0)
		{
			int len = it->get_path().size();
			int target_len = target.size();
			if (len > max_len && len <= target_len)
			{
				max_len = it->get_path().size();
				location = &(*it);
			}
		}
	}
	return (location);
}

void	set_response(Server& server, request_t* request, response_t* response)
{
	Location						*location = NULL;
	// check if there is location block inside the choosen server
	// then we check if the target is inside the location block (exactly)
	// then we crop the uri name to find if an inexact location's name fit with a part of the URI
	if (server.get_locations().size() == 0)
		choose_return_code_for_requested_ressource(server, request, response);
	else
		location = choose_location(server, request, response);
	if (location == NULL)
		choose_return_code_for_requested_ressource(server, request, response);
	else
		set_location_options(server, request, *location, response);
	
	/* call function to create response(return_code, location, server) */
	std::cout << "The choosen location is : " << location->get_path() << std::endl;
}

// int main(void)
// {
// 	request_t req;
// 	Location loc("/author/articles/subject/");;
// 	Location loc4("/author/articles/subject");;
// 	Location loc2("/author/articles/subje");
// 	Location loc3("/author/articles/su");
// 	std::pair<uint16_t, std::string> ret;
// 	ret.first = 404;
// 	ret.second = "/404.html";
// 	req.method = "GET";
// 	req.target = "/author/articles/subject/index.html";
// 	Server server;
// 	loc4.get_error_pages().push_back(ret);
// 	std::cout << loc4.return_codes.err_404 << std::endl;
// 	server.get_auto_index() = true;
// 	server.get_index_file() = "/index.html";
// 	server.get_root_path() = DEFAULT_ROOT_PATH;
// 	server.get_locations().push_back(loc3);
// 	server.get_locations().push_back(loc4);
// 	server.get_locations().push_back(loc2);
// 	server.get_locations().push_back(loc);

// 	set_location_block(server, req);
// }
