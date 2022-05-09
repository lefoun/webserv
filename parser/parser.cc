#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include "Server.hpp"

enum TOKEN {
	UNKNOWN_TOKEN = -1,
	PORT = 0,
	IP,
	DOMAIN_NAME,
	PATH,
	DIRECTIVE,
	KEY_WORD,
	OPEN_BRACKET,
	CLOSED_BRACKET,
	SEMI_COLON
};

enum DIRECTIVES {
	SERVER = 0,
	LISTEN,
	SERVER_NAME,
	ROOT,
	INDEX,
	CGI_PY,
	CGI_PHP,
	AUTO_INDEX,
	LOCATION,
	UNKNOWN_DIRECTIVE,
	DIRECTIVES_NB
};

/* Config file parsing
 * Input is a  file
 * need to open it and have it as a stream
 * then read the stream and generate tokens out of it
 * if error return error else
 * verify if the syntaxe of the config is correct
 * if no return error
 * else create servers
 * launch main loop
*/

/* En parsant le fichier on doit trouver:
	* Directive[server] ... 
	* server : Listen 
*/

// int	get_directive(const std::string& directive)
// {
// 	return UNKNOWN_DIRECTIVE;
// }

const std::vector<std::string> init_directives()//std::vector<std::string>& directives)
{
	std::vector<std::string> directives(DIRECTIVES_NB);

	directives[SERVER] = "server";
	directives[LISTEN] = "listen";
	directives[SERVER_NAME] = "server_name";
	directives[ROOT] = "root";
	directives[INDEX] = "index";
	directives[CGI_PY] = "cgi_py";
	directives[CGI_PHP] = "cgi_php";
	directives[AUTO_INDEX] = "auto_index";
	return directives;
}

int	find_directive(const std::vector<std::string>& directives,
					const std::string& token)
{
	for (ssize_t i = 0; i < DIRECTIVES_NB; ++i)
		if (directives[i] == token)
			return i;
	return UNKNOWN_DIRECTIVE;
}

void	handle_listen(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{

(void)token; (void)context; (void)server;
}

void	handle_server_name(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{

(void)token; (void)context; (void)server;
}

void	handle_root(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{

(void)token; (void)context; (void)server;
}

void	handle_index(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{

(void)token; (void)context; (void)server;
}

void	handle_cgi_py(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{

(void)token; (void)context; (void)server;
}

void	handle_cgi_php(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	(void)token; (void)context; (void)server;
}

void	handle_auto_index(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	(void)token; (void)context; (void)server;
}

void	handle_location(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	(void)token; (void)context; (void)server;
}
void	get_server(std::istream_iterator<std::string>& token, Server& server,
					std::stack<std::string>& context,
					const std::vector<std::string>& directives_vec)
{

	const std::istream_iterator<std::string> end_of_file;
	if (*(++token) != "{")
		throw std::invalid_argument("Excpected token '{'");
	++token;

	int directive;
	while (true)
	{
		if (*token == "}")
			return ;
		else if (token == end_of_file)
			throw std::invalid_argument("Excepted token '}'");

		directive = find_directive(directives_vec, *token);
		std::cout << "This is token " << *token << std::endl;
		if (directive == UNKNOWN_DIRECTIVE)
			std::cout << "Not found " << *token<< "\n";
		else
		{
			std::cout << "Found " << *token << std::endl;
			switch (directive)
			{
				case SERVER:
					throw std::invalid_argument("Found nested servers");
				case LISTEN:
					handle_listen(token, context, server); break;
				case SERVER_NAME:
					handle_server_name(token, context, server); break;
				case ROOT:
					handle_root(token, context, server); break;
				case INDEX:
					handle_index(token, context, server); break;
				case CGI_PY:
					handle_cgi_py(token, context, server); break;
				case CGI_PHP:
					handle_cgi_php(token, context, server); break;
				case AUTO_INDEX:
					handle_auto_index(token, context, server); break;
				case LOCATION:
					handle_location(token, context, server); break;
			}
		}
		++token;
	}
}

bool	parse_config_file(const std::string& file_name)
{
	std::ifstream	config_file(file_name);
	if (!config_file.is_open() || config_file.fail())
		throw config_file.exceptions();

	std::vector<Server>	servers;
	std::stack<std::string> context;
	std::istream_iterator<std::string> token(config_file);
	const std::istream_iterator<std::string> end_of_file;
	const std::vector<std::string> directives = init_directives();

	while (token != end_of_file)
	{
		if (*token == "server")
		{
			context.push("server");
			servers.push_back(Server());
			try
			{
				get_server(token, servers.back(), context, directives);
				context.pop();
			}
			catch (std::exception& e)
			{
				std::cout << "Failed to parse config file\n" << e.what() << "\n";
				return false;
			}
		}
		++token;
	}
	return false;	
}

int main()
{
	if (parse_config_file("server_config.conf"))
		std::cout << "File is good\n";
	else
		std::cout << "Error\n";
	return 0;
}


// void load_server_config(Server& config) {
//     std::ifstream fin("config.txt");
//     string line;
//     while (getline(fin, line)) {
//         istringstream sin(line.substr(line.find("=") + 1));
//         if (line.find("num") != -1)
//             sin >> config.num;
//         else if (line.find("str") != -1)
//             sin >> config.str;
//         else if (line.find("flt") != -1)
//             sin >> config.flt;
//     }
// }

// int main() {
//     Config config;
//     loadConfig(config);
//     cout << config.num << '\n';
//     cout << config.str << '\n';
//     cout << config.flt << '\n';
// }
