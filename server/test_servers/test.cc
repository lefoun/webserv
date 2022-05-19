#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <algorithm>
#include "Server.hpp"
#include <iostream>
#include <stdlib.h>
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

int main () {
   cout << "Content-type:text/html\r\n\r\n";
   cout << "<html>\n";
   cout << "<head>\n";
   cout << "<title>CGI Environment Variables</title>\n";
   cout << "</head>\n";
   cout << "<body>\n";
   cout << "<table border = \"0\" cellspacing = \"2\">";

   for ( int i = 0; i < 24; i++ ) {
      cout << "<tr><td>" << ENV[ i ] << "</td><td>";
      
      // attempt to retrieve value of environment variable
      char *value = getenv( ENV[ i ].c_str() );  
      if ( value != 0 ) {
         cout << value;                                 
      } else {
         cout << "Environment variable does not exist.";
      }
      cout << "</td></tr>\n";
   }
   
   cout << "</table><\n";
   cout << "</body>\n";
   cout << "</html>\n";
   
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
