#pragma once

#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <stack>
#include <dirent.h>
#include "helper_functions.hpp"
#include "Server.hpp"
#include "webserver.hpp"

enum DIRECTIVES {
	UNKNOWN_DIRECTIVE = -1,
	SERVER,
	LISTEN,
	SERVER_NAME,
	ROOT,
	INDEX,
	AUTO_INDEX,
	LOCATION,
	ERROR_DIRECTIVE,
	REDIRECTION,
	ALLOW,
	CLIENT_MAX_BODY_SIZE,
	DIRECTIVES_NB
};
