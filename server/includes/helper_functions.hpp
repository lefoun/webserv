#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

/* for ip_to_number function*/
#define INVALID 0

template <typename U>
bool    is_in_vector(const std::vector<U>& vect, const U& value)
{
	if (vect.empty())
		return false;
	return std::find(vect.begin(), vect.end(), value) != vect.end();
}

template <typename T>
bool	        in_range(T low, T high, T num);

bool	        is_number(const std::string& s);

bool	        is_ip_address(const std::string &ip_str);

in_addr_t	ip_to_number(const char * ip);

std::string	ip_to_str(in_addr_t ip);
/* Reads size from buffer or till buffer[index] == '\0' if no size was given */
void	read_buf(char buffer[], size_t size = 0);
