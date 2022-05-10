#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include "Server.hpp"

template <typename T>
static bool	in_range(T low, T high, T num)
{
	return num >= low && num <= high;
}

static bool is_number(const std::string& s)
{
    if(s.size() == 0)
		return false;
    for (size_t i = 0; i < s.size(); i++)
	{
        if ((s[i]>= '0' && s[i] <='9') == false)
            return false;
    }
    return true;
}

static bool	is_ip_address(const std::string &ip_str)
{
	if (std::count(ip_str.begin(), ip_str.end(), '.') != 3)
		return false;

	std::stringstream			ip_ss(ip_str);
	std::string					split_ip;
	std::vector<std::string>	ip_octet_holder;
	while (std::getline(ip_ss, split_ip, '.'))
	{
		std::cout << split_ip << std::endl;
		ip_octet_holder.push_back(split_ip);
		std::string octect = ip_octet_holder.back();
		if (!is_number(octect) || octect.size() > 3
			|| !in_range(0, 255, atoi(octect.c_str())))
			return false;
	}
	if (ip_octet_holder.size() != 4)
		return false;
	return true;
}


int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	std::cout << std::boolalpha;
	std::string	root;
	std::string non_root("");
	std::cout << root.empty() << std::endl;
	std::cout << non_root.empty() << std::endl;
	std::cout << is_ip_address(std::string(av[1])) << std::endl;
	// std::cout << is_valid_host_name(std::string(av[1]), Server()) << std::endl;
}
