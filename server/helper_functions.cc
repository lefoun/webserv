#include "helper_functions.hpp"

template <typename T>
bool	in_range(T low, T high, T num)
{
	return num >= low && num <= high;
}

bool is_number(const std::string& s)
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

bool	is_ip_address(const std::string &ip_str)
{
    //std::count n'existe pas en c++ 98
	int count = 0;
	for (size_t i = 0; i < ip_str.size(); i++)
	{
		if (ip_str[i] == '.')
			count++;
	}
	if (count != 3)
		return false;

	std::stringstream			ip_ss(ip_str);
	std::string					split_ip;
	std::vector<std::string>	ip_octet_holder;
	while (std::getline(ip_ss, split_ip, '.'))
	{
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

in_addr_t	ip_to_number(const char * ip)
{
    /* The return value. */
	in_addr_t	v = 0;
    /* The count of the number of bytes processed. */
	int i;
    /* A pointer to the next digit to process. */
	const char * start;

    start = ip;
    for (i = 0; i < 4; i++) {
        /* The digit being processed. */
        char c;
        /* The value of this byte. */
        int n = 0;
        while (1) {
            c = * start;
            start++;
            if (c >= '0' && c <= '9') {
                n *= 10;
                n += c - '0';
            }
            /* We insist on stopping at "." if we are still parsing
               the first, second, or third numbers. If we have reached
               the end of the numbers, we will allow any character. */
            else if ((i < 3 && c == '.') || i == 3) {
                break;
            }
            else {
                return INVALID;
            }
        }
        if (n >= 256) {
            return INVALID;
        }
        v *= 256;
        v += n;
    }
    return v;
}

void	read_buf(char buffer[], size_t size)
{
	if (size == 0)
	{
		for (size_t i = 0; buffer[i] != '\0'; ++i)
			std::cout << buffer[i];
	}
	else
	{
		for (size_t i = 0; i < size; ++i)
			std::cout << buffer[i];
		std::cout << std::endl;
	}
}
