#pragma once

#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include "Server.hpp"

enum REQUEST_METHODS {
	GET = 0,
	POST,
	DELETE
};

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
