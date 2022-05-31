#include "../includes/webserver.hpp"

void	set_cgi_env_variables(const request_t* request)
{
	setenv("CONTENT_TYPE", request->content_type.c_str(), 1);
	setenv("REDIRECT_STATUS", "200", 1);
	if (request->method == "POST")
		setenv("CONTENT_LENGTH", SSTR(request->content_length).c_str(), 1);
	else
		setenv("CONTENT_LENGTH", "", 1);
	setenv("HTTP_COOKIE", request->permanent_cookie.c_str(), 1);
	setenv("SESSION_COOKIE", request->session_cookie.c_str(), 1);
	setenv("PATH_INFO", request->path_info.c_str(), 1);
	setenv("PATH_TRANSLATED", request->path_info.c_str(), 1);
	setenv("QUERY_STRING", request->query_string.c_str(), 1);
	setenv("REMOTE_ADDR", request->remote_addr.c_str(), 1);
	setenv("REMOTE_HOST", request->remote_host.c_str(), 1);
	setenv("REQUEST_METHOD", request->method.c_str(), 1);
	setenv("REQUEST_", request->path_info.c_str(), 1);
	setenv("SCRIPT_FILENAME", request->path_info.c_str(), 1);
	setenv("SCRIPT_NAME", request->path_info.c_str(), 1);
	setenv("SERVER_NAME", request->host.c_str(), 1);
	setenv("CONNECTION", request->connection.c_str(), 1);
	setenv("SERVER_SOFTWARE", "WebServ", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	setenv("BUFFER_SIZE", SSTR(BUFFER_SIZE).c_str(), 1);
}

void	get_cgi_response(const request_t* request, response_t* response,
							std::string& response_str, const int& socket_fd)
{
	pid_t child_pid = 1;
	std::string	file_name = "cgi-bin/cgi_serv_communication_file.txt";

	child_pid = fork();

	if (child_pid < 0)
	{
		std::cout << "Failed to create a new process\n";
		return ;
	}
	if (child_pid == 0) /* Child process */
	{
		extern char **environ;
		char *args[3];
		args[0] = const_cast<char *const>(request->path_info.c_str());
		args[1] = NULL;
		if (request->method == "POST")
		{
			std::ofstream cgi_communication_file(file_name.c_str());
			if (cgi_communication_file.fail())
				throw std::runtime_error(
					"Failed to send a POST request to CGI");
			else
			{
				cgi_communication_file << request->body;
				cgi_communication_file.close();
			}
		}
		set_cgi_env_variables(request);
		execve(*args, args + 1, environ);
		exit(0);
	}
	else /* parent */
	{
		wait(NULL);
		std::ifstream cgi_output_file(file_name.c_str());
		if (cgi_output_file.fail())
			throw std::runtime_error("Failed to send a response from CGI");
		std::stringstream tmp;
		tmp << cgi_output_file.rdbuf();
		cgi_output_file.close();
		response_str = tmp.str();
		if (response_str.size() >= BUFFER_SIZE)
		{
			send_chunked_response(response, response_str, socket_fd);
			return ;
		}
		if (send(socket_fd, response_str.c_str(), response_str.size(), 0) < 0)
			throw std::runtime_error(
				"Failed to send data to socket " + SSTR(socket_fd));
	}
}



void	get_cgi_php_response(request_t* request, response_t* response,
							std::string& response_str, const int& socket_fd)
{
	std::string file_name = "cgi-bin/cgi_serv_communication_file.txt";
	FILE *body = std::tmpfile();
	std::string	php = "/usr/bin/php-cgi";
	fputs(request->body.c_str(), body);
	int fd_restore_in = dup(STDIN_FILENO);
	std::rewind(body);
	int fd_body = fileno(body);
	pid_t pid= fork();

	if (body == NULL)
		throw std::runtime_error("Failed to create a temporary file");
	if (pid < 0)
		throw std::runtime_error("Failed to create a new process");
	else if (pid == 0)
	{
		char *args[2];
		args[0] =  const_cast<char *const>(php.c_str());
		args[1] = NULL;
		set_cgi_env_variables(request);
		extern char **environ;
		if (dup2(fd_body, STDIN_FILENO) == -1)
			throw (std::runtime_error("System error : Failed to redirect STDIN\n"));
		close(fd_body);
		close(fd_restore_in);
		fclose(body);
		execve(*args, args + 1, environ);
		exit(0);
	}
	else
	{
		wait(NULL);
		std::ifstream cgi_output_file(file_name.c_str());
		if (cgi_output_file.fail())
			throw std::runtime_error("Failed to send a response from CGI");
		std::stringstream tmp;
		tmp << cgi_output_file.rdbuf();
		cgi_output_file.close();
		response_str = tmp.str();
		if (response_str.size() >= BUFFER_SIZE)
		{
			send_chunked_response(response, response_str, socket_fd);
			return ;
		}
		if (send(socket_fd, response_str.c_str(), response_str.size(), 0) < 0)
			throw std::runtime_error(
				"Failed to send data to socket " + SSTR(socket_fd));
	}
	dup2(fd_restore_in, STDIN_FILENO);
	close(fd_restore_in);
	close(fd_body);
	fclose(body);
}


