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

std::string ip_to_str(in_addr_t ip)
{
    struct sockaddr_in sa;
    char str[INET_ADDRSTRLEN];

    sa.sin_addr.s_addr = htonl(ip);
    return std::string(inet_ntop(AF_INET, &sa.sin_addr, str, INET_ADDRSTRLEN));
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
		for (size_t i = 0; (int)(buffer[i]) != '\0'; ++i)
			std::cout << buffer[i] << "|";
	}
	else
	{
		for (size_t i = 0; i < size; ++i)
			std::cout << (int)(buffer[i]) << "|";
		std::cout << std::endl;
	}
}

std::string get_current_time()
{
    time_t now = time(0);
    struct tm *tstruct = gmtime(&now);
    tstruct->tm_year += 1;
    char *date = asctime(tstruct);
    std::string str(date);
    str[str.size() - 1] = ' ';
    str.append("GMT");
    return (str);
}

int remove_files_and_dir(const char *fpath, const struct stat *sb,
                   int typeflag, struct FTW *ftwbuf)
{
    std::cout << fpath << " " << typeflag << std::endl;
    (void)ftwbuf;
    (void)sb;
 /*   if (typeflag == FTW_F)
        if (unlink(fpath) == -1)
            return (-1);
    if (typeflag == FTW_D)

        if (rmdir(fpath) == -1)
            return (-1);*/
    // if (remove(fpath))
    //     return -1;
    return 0;
}

int remove_dir(const char* dir_path)
{

    return nftw(dir_path, remove_files_and_dir, 64, FTW_DEPTH);
}
