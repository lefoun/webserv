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

static bool is_number(const string& s)
{
    if(s.size() == 0)
		return false;
    for (size_t i = 0; i < s.size(); i++) {
        if ((s[i]>='0' && s[i]<='9') == false) {
            return false;
        }
    }
    return true;
}

static void	set_port(const std::string& port_str, Server& server)
{
	int	port_num;
	if (port_str.size() > 5)
		throw std::invalid_argument("Port number out of range");
	port_num = atoi(port_str.c_str());
	if (port_num > 65535)
		throw std::invalid_argument("Invalid Port number");
	(server.get_listening_ports()).push_back(atoi(port_str.c_str());
}

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

static void	set_ip(const std::string& host, Server& server)
{
	/* if host == an ip address -> le mettre directement dans get_listening ip*/
	/* sinon si c'est un host -> ouvrir le fichier /etc/host et verif si il exist
	si oui prendre l'ip correspodente et la mettre dedans sinon throw */

	ssize_t pos = 0;
	ssize_t i = 0;
	uint16_t nb_octets = 0;
	while (nb_octets < 4){ 
		pos = host.find('.', i);
		if (pos == std::string::npos && nb_octets != 3)
			throw std::invalid_argument("Find invalid ip format."); //si on ne trouve pas de '.' et que l'adresse n'a pas 4 octet;
		++nb_octets;
		std::string octet = host.substr(i, pos - i);
		if (atoi(octet.c_str()) < 0 || atoi(octet.c_str()) > 255) // si l'octet est inferieur a 0 ou superieur a 255
			throw std::invalid_argument("Find invalid ip format.");
		i = (pos + 1);
		octet.clear();
	//	std::cout << "octet =" << octet << " i =" << i  << std::endl;
	}
	std::cout<< "ip ok" << std::endl;

	std::ifstream	hosts_file("/etc/hosts");	//open hosts file
	std::istream_iterator<std::string> parser_hosts(hosts_file);
	std::istream_iterator<std::string> end_of_file;

	while (parser_hosts != end_of_file){//on check si l'ip est dans le ficher host
		if(*parser_hosts == host){
			(server.get_listening_ips()).push_back(host.c_str());
			return;
		}
		++parser_hosts;		
	}
	throw std::invalid_argument("Unable to find host.");

}

void	handle_listen(std::istream_iterator<std::string>& token,
						std::stack<std::string>& context, Server& server)
{
	(void)context;
	std::istream_iterator<std::string> end_of_file;
	if (++token == end_of_file)
		throw std::invalid_argument("Unexpected end of file");
	if (*(--(*token).end()) != std::string(";"))	
		throw std::invalid_argument("Excpected token ';'");
	token->erase(token->size() - 1);
	/* Listen either to a Host, a Port number or a Host:Port pair */
	std::string::size_type	pos = token->find(":");
	if (pos == std::string::npos) /* Means that it's either a Host or a Port */
	{
		if (is_number(*token))
			set_port(*token, server);
		else
			set_ip(*token, server);
	}
	else
	{
		std::pair<uint16_t, std::string>	ip_port_pair;
		std::string	tmp_port = token->substr(0, pos);
		std::string	tmp_host = token->substr(pos, std::string::npos);
		set_port(tmp_port, server);
		set_ip(tmp_host, server);
	}
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
