#pragma once

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

in_addr_t		ip_to_number(const char * ip);

std::string		ip_to_str(in_addr_t ip);
/* Reads size from buffer or till buffer[index] == '\0' if no size was given */
void			read_buf(char buffer[], size_t size = 0);

std::string		get_current_time(int years);

int remove_dir(const char* dir_path);

int remove_files_and_dir(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

std::string get_body_auto_index(std::string full_path, std::string dir_path);
