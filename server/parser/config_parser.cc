#include "config_parser.hpp"

const std::vector<std::string> init_directives()
{
	std::vector<std::string> directives(DIRECTIVES_NB);

	directives[SERVER] = "server";
	directives[LISTEN] = "listen";
	directives[SERVER_NAME] = "server_name";
	directives[ROOT] = "root";
	directives[INDEX] = "index";
	directives[AUTO_INDEX] = "auto_index";
	directives[LOCATION] = "location";
	directives[ERROR_DIRECTIVE] = "error_page";
	directives[REDIRECTION] = "redirect";
	directives[ALLOW] = "allow";
	directives[CLIENT_MAX_BODY_SIZE] = "client_max_body_size";
	return directives;
}



void	init_host_ip_lookup(std::map<std::string, std::string>& host_ip_lookup)
{

	std::ifstream	hosts_file("/etc/hosts");
	if (hosts_file.fail())
		throw std::invalid_argument("Failed to open hosts file");

	std::string	line;
	while (std::getline(hosts_file, line))
	{
		if (std::isdigit(line[0]) && std::isdigit(line[0]) != 2)
		{
			std::istringstream tmp(line);
			std::string	ip_addr;
			std::string	host_name;
			tmp >> ip_addr;
			tmp >> host_name;
			for (std::map<std::string, std::string>::const_iterator it =
				host_ip_lookup.begin(); it != host_ip_lookup.end(); ++it)
			{
				if (host_name == it->first)
					return ;
			}
			host_ip_lookup.insert(std::make_pair(host_name, ip_addr));
		}
	}
	hosts_file.close();
}


void	check_errors(std::vector<Server>& servers)
{
	std::vector<Server>::iterator it = servers.begin();
	std::vector<ips_ports_server_names>	ips_ports_names; //vector containing a pair of ip/pair and a vector of server_names
	std::vector< std::string > location_path;//this vector is just for checking the duplicate location with same path

	//i iterate throw the servers and fill ips_ports_names with all the implicit and explicit pairs of ips/ports
	//and the server_names of each server, and i check if i find same ip/port with same server_name, witch throws an error;
	std::vector<ips_ports_server_names>::iterator it_ips_ports_names;
	while (it != servers.end())
	{
		for (std::vector<uint16_t>::iterator it_listen_ports = it->
			get_listening_ports().begin();	it_listen_ports != it->
			get_listening_ports().end(); ++it_listen_ports)
		{
			for (it_ips_ports_names = ips_ports_names.begin();
				it_ips_ports_names != ips_ports_names.end(); ++it_ips_ports_names)
				if (it_ips_ports_names->first ==
						std::make_pair(std::string("127.0.0.1"), *it_listen_ports) &&
						is_duplicate(it_ips_ports_names->second, it->get_server_names()))
					throw (std::invalid_argument("Error: found duplicate server_name for same port"));
			ips_ports_names.push_back(std::make_pair(std::make_pair
				("127.0.0.1", *it_listen_ports),
			it->get_server_names()));
		}
		for(std::vector<Server::ip_port_pair>::iterator it_ip_port_pair = it->
			get_ip_port_pairs().begin(); it_ip_port_pair != it->
			get_ip_port_pairs().end(); ++it_ip_port_pair)
		{
			for (it_ips_ports_names = ips_ports_names.begin();
			it_ips_ports_names != ips_ports_names.end(); ++it_ips_ports_names)
				if (*it_ip_port_pair == it_ips_ports_names->first && is_duplicate
						(it_ips_ports_names->second, it->get_server_names()))
					throw (std::invalid_argument("Error: found duplicate server_name for listening directive"));
			ips_ports_names.push_back(std::make_pair(*it_ip_port_pair,
				it->get_server_names()));
		}
		for (std::vector<std::string>::iterator it_listen_ips = it->
			get_listening_ips().begin(); it_listen_ips != it->
			get_listening_ips().end(); ++it_listen_ips)
		{
			for (it_ips_ports_names = ips_ports_names.begin();
				it_ips_ports_names != ips_ports_names.end(); ++it_ips_ports_names)
				if (it_ips_ports_names->first == std::make_pair(*it_listen_ips,
						uint16_t(8000)) && is_duplicate(it_ips_ports_names->second,
						it->get_server_names()))
					throw (std::invalid_argument("Error: found duplicate server_name for same ip"));
			ips_ports_names.push_back(std::make_pair(std::make_pair
				(*it_listen_ips, uint16_t(8000)), it->get_server_names()));
		}
		//same way for the locations, i fill the vector location_path and look for duplicates
		for (std::vector<Location>::iterator it_location = it->get_locations()
			.begin(); it_location !=  it->get_locations().end(); ++it_location)
		{
			if (it_location->is_location_empty()) //checking if the location is not empty
				throw (std::invalid_argument("Error: found empty location"));
			for (std::vector<Location>::iterator it_location2 = it_location + 1;
				it_location2 != it->get_locations().end(); ++it_location2)
			{
				if (it_location->get_path() == it_location2->get_path()
						&& it_location != it_location2 )
					throw (std::invalid_argument("Error: found duplicate location"));
			}
		}
		it++;
	}

}

void	enriche_configuration(std::vector<Server>& servers,
									std::map<std::string, std::string>&
									host_ip_lookup)
{
	for (std::vector<Server>::iterator it = servers.begin();
			it != servers.end(); ++it)
	{
		it->set_host_lookup_map(&host_ip_lookup);
		if (it->get_allowed_methods().empty())
			set_default_methods(*it);
		if (it->get_index_file().empty())
			it->set_index_file("index.html");
		if (it->get_is_auto_index_set() == false)
			it->set_auto_index(false);
		if (it->get_root_path().empty())
		{
			try
			{
				DIR *dir = opendir("./www"); // peut-etre trouver une meilleure facon de trouver le dossier www plutot que de faire un truc de schlag avec le chemin relatif
				if (dir)
				{
					it->set_root_path("./www");
					closedir(dir);
				}
				else
					throw std::invalid_argument("You probably erased the default root path damn stupid.");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				exit(1);
			}
		}
		set_default_return_code(*it);
		for (size_t i = 0; i < it->get_locations().size(); ++i)
		{
			if (it->get_locations()[i].get_allowed_methods().empty())
				set_default_methods(it->get_locations()[i]);
			if (it->get_locations()[i].get_root_path().empty())
				it->get_locations()[i].set_root_path(it->get_root_path());
			if (it->get_locations()[i].get_index_file().empty())
				it->get_locations()[i].set_index_file("index.html");
			if (it->get_locations()[i].get_is_auto_index_set() == false)
				it->get_locations()[i].set_auto_index(false);
			set_default_return_code(it->get_locations()[i]);
		}
		if (it->get_listening_ips().empty() && it->get_listening_ports().empty()
			&& it->get_ip_port_pairs().empty())
			servers.begin()->get_ip_port_pairs().push_back(
				std::make_pair("127.0.0.1", 8000));
	}
	set_implicit_ip_port_pairs(servers);
}

bool	parse_config_file(const std::string& file_name,
							std::vector<Server>& servers,
							std::map<std::string, std::string>& host_ip_lookup)
{
	init_host_ip_lookup(host_ip_lookup);
	std::ifstream	config_file(file_name.c_str());
	if (!config_file.is_open() || config_file.fail())
		throw config_file.exceptions();

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
				get_server(token, servers.back(), context, directives,
							host_ip_lookup);
				context.pop();
			}
			catch (std::exception& e)
			{
				std::cout << "Failed to parse config file\n"
						<< e.what() << "\n";
				return false;
			}
		}
		++token;
	}
	if (servers.empty())
		throw std::invalid_argument("Empty config file");
	check_errors(servers);
	enriche_configuration(servers, host_ip_lookup);
	config_file.close();
	return true;
}
