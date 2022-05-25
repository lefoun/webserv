#pragma once

#include <iostream>
#include <sys/types.h>
#include "request_parsing.hpp"

typedef struct
{
	uint8_t			response_state;
	uint16_t		return_code;
	bool			is_auto_index;
	bool			is_chunked;
	std::string		date;
	std::string		content_type;
	std::string		location;
	std::string		body;
	std::string		file_path;
	std::string		server;
} response_t;
