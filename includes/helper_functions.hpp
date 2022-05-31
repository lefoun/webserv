#pragma once

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <ftw.h>
#include <dirent.h> 
#include <stdio.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h> 
#include "webserver.hpp"
#include "request_parsing.hpp"



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

in_addr_t		ip_to_number(const char * ip);

std::string		ip_to_str(in_addr_t ip);

void			read_buf(char buffer[], size_t size = 0);

std::string		get_current_time(int years);

int remove_dir(const char* dir_path);

int remove_files_and_dir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

char	get_char_from_hex(const std::string& str, const size_t& index, const size_t& str_size);

void	check_char_in_stream(const char& delimiter, std::istringstream& ss);

void		print_request_content(const request_t& request);
