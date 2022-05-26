#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <ctime>
using namespace std;

const string ENV[ 24 ] = {
   "COMSPEC", "DOCUMENT_ROOT", "GATEWAY_INTERFACE",   
   "HTTP_ACCEPT", "HTTP_ACCEPT_ENCODING",             
   "HTTP_ACCEPT_LANGUAGE", "HTTP_CONNECTION",         
   "HTTP_HOST", "HTTP_USER_AGENT", "PATH",            
   "QUERY_STRING", "REMOTE_ADDR", "REMOTE_PORT",      
   "REQUEST_METHOD", "REQUEST_URI", "SCRIPT_FILENAME",
   "SCRIPT_NAME", "SERVER_ADDR", "SERVER_ADMIN",      
   "SERVER_NAME","SERVER_PORT","SERVER_PROTOCOL",     
   "SERVER_SIGNATURE","SERVER_SOFTWARE" };   

/*
Thu, 26 May 2022 16:14:00 GMT
*/
std::string get_current_time(int years)
{
   (void)years;
   char    output[100];
   std::string str;
   time_t  raw_time;
   std::time(&raw_time);
   struct  tm *tstruct = std::gmtime(&raw_time);
   tstruct->tm_year += years;
   std::strftime(output, sizeof(output), "%a, %d %b %G %T GMT", tstruct);
   str = output;
   return (str);
}
int main () {
 std::cout <<  get_current_time(0) << std::endl; 
   return 0;
}

// une fonction qui

// int main(int ac, char **av)
// {
// 	(void)ac;
// 	(void)av;
// 	std::cout << std::boolalpha;
// 	std::string	root;
// 	std::string non_root("");
// 	std::cout << root.empty() << std::endl;
// 	std::cout << non_root.empty() << std::endl;
// 	std::cout << is_ip_address(std::string(av[1])) << std::endl;
// 	// std::cout << is_valid_host_name(std::string(av[1]), Server()) << std::endl;
// }
