#include "../includes/webserver.hpp"

std::string	generate_cookie(const size_t size)
{
	static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;

	tmp_s.reserve(size);
	srand(time(NULL));
	for (size_t i = 0; i < size; ++i)
	tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	return tmp_s;
}


void	send_chunked_response(response_t* response, std::string& response_str,
								const int& socket_fd)
{
	bool	contains_header = true;

	while (true)
	{
		char hex_num[20];
		memset(hex_num, 0, 20);
		if (response_str.size() < BUFFER_SIZE)
			sprintf(hex_num, "%lX", response_str.size());
		else
			sprintf(hex_num, "%X", BUFFER_SIZE);

		std::string chunk_len_line(std::string(hex_num) + std::string(CRLF));
		if (contains_header)
			response_str.insert(response_str.find(DOUBLE_CRLF) + 4,
								chunk_len_line); /* Insert after the header */
		else
			response_str.insert(0, chunk_len_line);

		std::string::size_type chunk_len_line_pos = response_str.find(chunk_len_line);

		std::string rsp_str;
		if (contains_header)
			 rsp_str = response_str.substr(0,
						chunk_len_line_pos + chunk_len_line.size() + BUFFER_SIZE);
		else if (response_str.size() > BUFFER_SIZE)
			rsp_str = response_str.substr(
				0, chunk_len_line.size() + BUFFER_SIZE);
		else
			 rsp_str = response_str.substr(
				 0, chunk_len_line.size() + response_str.size());

		if (rsp_str.size() < BUFFER_SIZE)
		{
			rsp_str.append("\r\n0\r\n\r\n"); /* End of response */
			response->response_state = COMPLETE;
		}
		else
			rsp_str.append("\r\n");

		if (send(socket_fd, rsp_str.c_str(), rsp_str.size(), 0) < 0)
			throw std::runtime_error("Failed to send data to socket " +
										SSTR(socket_fd));
		if (response->response_state == COMPLETE)
			return ;
		if (contains_header)
			response_str = response_str.substr(
				chunk_len_line_pos + chunk_len_line.size() + BUFFER_SIZE);
		else
			response_str = response_str.substr(BUFFER_SIZE);
		contains_header = false;
	}
}

void	construct_header(response_t* response, request_t* request,
							std::string& header)
{
	header.reserve(200);
	header.append("HTTP/1.1 ");
	header.append(SSTR(response->return_code));
	header.append(" ");
	header.append(response->return_message + CRLF);
	if (response->return_code != 302)
	{
		std::string content_type = get_content_type(
			request->target.substr(request->target.find_last_of(".") + 1));
		header.append("content-type: " );
		response->content_type = content_type;
		header.append(content_type.append(CRLF));
		header.append("Connection: keep-alive\r\n");
	}
	if (!response->is_chunked)
	{
		header.append("content-length: ");
		header.append(SSTR(response->body.size()) + CRLF);
	}
	if (response->is_chunked)
		header.append("Transfer-Encoding: chunked\r\n");
	header.append("date: ");
	header.append(response->date.append(CRLF));
	if (response->return_code == 302)
	{
		header.append("location: ");
		header.append(response->location.append(CRLF));
	}
		/* If the user visits an html page we track them */
	if (request->permanent_cookie.empty() && response->content_type == "text/html")
	{
		header.append("Set-Cookie: tracking-cookie=" + generate_cookie()
						+ "; Expires=" + get_current_time(1));
		header.append(CRLF);
	}
	header.append(CRLF);
}

std::string	get_content_type(const std::string& file_extension)
{
	std::string content_type = "text/html";
	if (file_extension == "css")
		content_type = "text/css";
	else if (file_extension == "jpeg")
		content_type = "image/jpeg";
	else if (file_extension == "jpg")
		content_type = "image/jpg";
	else if (file_extension == "js")
		content_type = "text/javascript";
	else if (file_extension == "ico")
		content_type = "image/png";
	return content_type;
}

void	send_response(request_t* request, const int& socket_fd,
						response_t* response)
{
	std::string	response_str;
	if (response->response_state == COMPLETE)
	{
		construct_header(response, request, response_str);
		if (!response->body.empty())
			response_str.append(response->body);
		if (send(socket_fd, response_str.c_str(), response_str.size(), 0) < 0)
			throw std::runtime_error(
				"Failed to send data to socket " + SSTR(socket_fd));
	}
	else /* Not started all Get/Post/Delete Requests that are valid (till now) */
	{
		std::string file_extension = request->target.substr(
									request->target.find_last_of(".") + 1);
		if (file_extension == "py")
		{
			std::cout << GREEN "Calling PYTHON-CGI " + file_extension + "\n" RESET;
			get_cgi_response(request, response, response_str, socket_fd);
			return ;
		}
		else if (file_extension == "php")
		{
			std::cout << GREEN "Calling PHP-CGI " + file_extension + "\n" RESET;
			get_cgi_php_response(request, response, response_str, socket_fd);
			return ;
		}
		else
		{
			if (file_extension == "html" && !request->session_cookie.empty())
			{
				std::cout << GREEN "Found Session Cookies: Calling CGI \n" RESET;
				get_cgi_response(request, response, response_str, socket_fd);
				return ;
				/* If the file is not download.html serve CGI output file */
				std::string cookie_file_path = "cgi-bin/cookies/"
												+ request->session_cookie + "_form";
				if (access(cookie_file_path.c_str(), R_OK) == -1)
					perror(cookie_file_path.c_str());
				else
					response->file_path = cookie_file_path;
			}
		}
		std::ifstream	file;
		if (response->content_type.find("image", 0) != std::string::npos)
			file.open(response->file_path.c_str(), std::ios::in | std::ios::binary);
		else
			file.open(response->file_path.c_str(), std::ios::in);
		if (file.fail())
			throw std::runtime_error("Failed to open file " + response->file_path);
		std::ostringstream tmp_ss;
		tmp_ss << file.rdbuf();
		response->body = tmp_ss.str();
		if (response->body.size() > BUFFER_SIZE)
		{
			response->is_chunked = true;
			construct_header(response, request, response_str);
			response_str.append(tmp_ss.str());
			send_chunked_response(response, response_str, socket_fd);
			file.close();
			return ;
		}
		construct_header(response, request, response_str);
		response_str.append(response->body);
		if (send(socket_fd, response_str.c_str(), response_str.size(), 0) < 0)
			throw std::runtime_error(
				"Failed to send data to socket " + SSTR(socket_fd));
		file.close();
	}
}

std::string get_body_auto_index(std::string full_path, std::string dir_path)
{
	DIR 			*d;
	struct dirent	*dir;
	struct stat		file_info;


	std::string body = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n \
		<meta charset=\"UTF-8\">\n<title>Index</title>\n</head>\n \
		<body><h1>Index of " + dir_path + "</h1>\n \
		<hr style=\"border-style: inset;\">\n";

	d = opendir(full_path.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL)
		{
			std::string path_to_file = full_path + "/" +  dir->d_name;
			if (stat(path_to_file.c_str(), &file_info) != - 1)
			{
				std::string len = SSTR(file_info.st_size);
				char *buf = new char[100];
				std::string name = dir->d_name;
				if (S_ISDIR(file_info.st_mode))
				{
					name = strcat(dir->d_name, "/");
					len = "_";
				}
				body.append("<div style=\"white-space: pre-wrap;\"><div style \
					=\"display: inline-block; min-width:180px;\"><a href=\"");
				body.append(name);
				body.append("\"; > ");
				body.append(name.c_str());
				body.append("</a></div>");
				struct tm *struct_time = gmtime(&file_info.st_mtime);
				strftime(buf, 200, "%e-%B-%Y %R",struct_time);
				sprintf(buf, "%-90s%s", buf, len.c_str());
				body.append (buf);
				body.append("</div>\n");
				delete [] buf;
			}

  		}
	}
	body.append("</hr></body>\n</html>");
	return body;
}
