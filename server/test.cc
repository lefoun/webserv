#include <iostream>
#include <unistd.h>

std::string get_current_time(int years)
{
	char		output[100];
	std::string	str;
	time_t		raw_time;
	std::time(&raw_time);
	struct tm *tstruct = std::gmtime(&raw_time);

	tstruct->tm_year += years;
	std::strftime(output, sizeof(output), "%a, %d %b %Y %H:%M:%S GMT", tstruct);
	str = output;
	return (str);
}

int main()
{
	std::cout << get_current_time(0) << std::endl;
	return 0;
}