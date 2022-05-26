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
						const int& return_code = 200,
						const std::string& return_message = "OK",
						const uint8_t& state = NOT_STARTED,
						const bool& is_auto_index = false,
						const std::string& location = "/",
						const std::string& file_path = "/",
						const std::string& body = "",
						const std::string& content_type = "text/html")
{
	response->return_code = return_code;
	response->return_message = return_message;
	response->is_auto_index = is_auto_index;
	response->date = get_current_time(0);
	response->content_type = content_type;
	response->location = location;
	response->body = body;
	response->response_state = state;
	response->file_path = file_path;
	response->is_chunked = false;

}

void	choose_return_code_for_requested_ressource(Server& server, request_t* request, response_t* response)
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
			return fill_response(response, 405, "Method Not Allowed" ,COMPLETE, false, "", "", server.return_codes.err_405);
		}
	}
	if (request->method == "GET")
	{
		std::string full_path = server.get_root_path() + request->target;
		DIR *dir = opendir(full_path.c_str());
		if (dir != NULL)
		{
			closedir(dir);
			if (*(--full_path.end()) != '/')
				return fill_response(response, 302, "Found" ,COMPLETE, false, request->target.append("/"));
			else
			{
				std::string index_file = full_path.append(server.get_index_file());
				if (access(index_file.c_str(), F_OK) == 0 && access(index_file.c_str(), R_OK) == 0)
					return fill_response(response, 200, "OK",NOT_STARTED, false, "", index_file);
				else if (server.get_auto_index() == true)
					return fill_response(response, 200, "OK", NOT_STARTED, true, "", full_path);
				else
					return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
			}
		}
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) == 0)
			return fill_response(response, 200, "OK",NOT_STARTED, false, full_path);
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) != 0)
			return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
		else
			return fill_response(response, 404, "Not Found",COMPLETE, false, "", "", server.return_codes.err_404);
	}
	if (request->method == "DELETE")
	{
		std::string full_path = server.get_root_path() + request->target;
		DIR *dir = opendir(full_path.c_str());
		if (dir != NULL)
		{
			closedir(dir);
			full_path.append(server.get_index_file());
			if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), W_OK) == 0)
			{
				if (remove(full_path.c_str()) != 0)
					fill_response(response, 500, "Internal Server Error",COMPLETE, false, "", "", server.return_codes.err_500);
				return fill_response(response, 200, "OK",NOT_STARTED, false, full_path);
			}
			else
				return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
		}
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), W_OK) == 0)
		{
			if (remove(full_path.c_str()) != 0)
				fill_response(response, 500, "Internal Server Error",COMPLETE, false, "", "", server.return_codes.err_500);
			return fill_response(response, 200, "OK",NOT_STARTED, false, full_path);
		}
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), W_OK) != 0)
			return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
		else
			return fill_response(response, 404, "Not Found",COMPLETE, false, "", "", server.return_codes.err_404);
	}
	fill_response(response, 501, "Not Implemented",COMPLETE, false, "", "", server.return_codes.err_501);
}

void	set_location_options(Server & server, request_t* request, Location & location, response_t* response)
{
	(void)response;
	bool autoindex = server.get_auto_index();

	std::cout << location.get_path() << std::endl;

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
			return fill_response(response, 405, "Method Not Allowed",COMPLETE, false, "", server.return_codes.err_405);
		}
	}
	if (request->method.compare("GET") == 0)
	{
		std::cout << "GET request->inside set_location_options" << std::endl;
		if (!location.get_redirections().empty())
		{
			std::string root = "/";
			if (location.get_redirections().at(0) == '/')
				root = "";
			std::string new_uri = request->target.erase(0, location.get_path().length());
			std::string new_url = root + location.get_redirections() + new_uri;
			std::cout << "302 REDIRECTION (new URL = " << new_url << " )" << std::endl;
			return fill_response(response, 302, "Found",COMPLETE, false, new_url);
		}
		std::string full_path = location.get_root_path() + request->target;
		DIR *dir = opendir(full_path.c_str());
		if (dir != NULL)
		{
			closedir(dir);
			if (*(--full_path.end()) != '/')
				return fill_response(response, 302, "Found",COMPLETE, false, request->target.append("/"));
			else
			{
				full_path.append(location.get_index_file());
				if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) == 0)
					return fill_response(response, 200, "OK",NOT_STARTED, false, "", full_path);
				else if (autoindex == true)
					return fill_response(response, 200, "OK",NOT_STARTED, true, "", full_path);
				else
					return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
			}
		}
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) == 0)
			return fill_response(response, 200, "OK",NOT_STARTED, false, "", full_path);
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), R_OK) != 0)
				return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
		else
			return fill_response(response, 404, "Not Found",COMPLETE, false, "", "", server.return_codes.err_404);
	}
	if (request->method.compare("DELETE") == 0)
	{
		std::cout << "DELETE request->inside set_location_options" << std::endl;
		std::string full_path = location.get_root_path() + request->target;
		DIR *dir = opendir(full_path.c_str());
		if (dir != NULL)
		{
			closedir(dir);
		//	full_path.append(index_file);
			if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), W_OK) == 0)
			{
				if (remove(full_path.c_str()) != 0)
					fill_response(response, 500, "Internal Server Error",COMPLETE, false, "", server.return_codes.err_500);
				return fill_response(response, 200, "OK",NOT_STARTED, false, full_path);
			}
			else
				return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
		}
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), W_OK) == 0)
		{
			if (remove(full_path.c_str()) != 0)
				fill_response(response, 500, "Internal Server Error",COMPLETE, false, "", server.return_codes.err_500);
			return fill_response(response, 200, "OK",NOT_STARTED, false, full_path);
		}
		else if (access(full_path.c_str(), F_OK) == 0 && access(full_path.c_str(), W_OK) != 0)
				return fill_response(response, 403, "Forbidden",COMPLETE, false, "", "", server.return_codes.err_403);
		else
			return fill_response(response, 404, "Not Found",COMPLETE, false, "", "", server.return_codes.err_404);
	}
	fill_response(response, 501, "Not Implemented",COMPLETE, false, "", "", server.return_codes.err_501);
}

/**
 * @brief Find in the server if a corresponding location exists for the requested URI.
 * @example URI = "/article/index.html", loc1 = /article, loc2 = /article/
 * @result loc2 is returned
 *
 * @param server
 * @param request
 * @param response
 * @return Location*
 */
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

/**
 * @brief Set the response object. If a location is found, we set the options for it.
 * Else, we set the options for the default server.
 *
 * @param server
 * @param request
 * @param response
 */
void	set_response(Server& server, request_t* request, response_t* response)
{
	Location						*location = NULL;
	if (server.get_locations().size() == 0)
		choose_return_code_for_requested_ressource(server, request, response);
	else
		location = choose_location(server, request, response);
	if (location == NULL)
		choose_return_code_for_requested_ressource(server, request, response);
	else
		set_location_options(server, request, *location, response);

	/* call function to create response(return_code, location, server) */
	if (location)
		std::cout << "The choosen location is : " << location->get_path() << std::endl;
}
