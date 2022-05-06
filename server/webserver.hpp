#pragma once

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <string>
#include <list>

class server
{
	private:
		const std::list<unsigned int>	_ports;
		const std::list<std::string>	_server_names;
		
};
