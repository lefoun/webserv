#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
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
	SERVER,
	LISTEN,
	SERVER_NAME,
	ROOT,
	INDEX,
	UNKNOWN_DIRECTIVE
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

bool	parse_config_file(const std::string& file_name)
{
	std::ifstream	config_file(file_name);

	if (!config_file.is_open() || config_file.fail())
		throw config_file.exceptions();
	std::istream_iterator<std::string>	it(config_file);
	std::vector<Server>	servers;
	while (true)
	{
		
	}

	return true;	
}

int main()
{
	// std::istream_iterator<std::string>	it(config_file);
	try 
	{
		parse_config_file("server_config.conf//");
		std::cout << "File is good\n";
	}
	catch (unsigned int)
	{
		std::cout << "File failed to open. Please provide a valid file.\n";
		return 1;
	}
	// std::cout << *it;
	// std::cout << *++it;
	// config_file >> hello_string;
	// std::cout << hello_string;
	return 0;
}

// struct Server{
//     int    		port;
//     std::string server_name;
//     std::string ip;
// };

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
