#include "request_parsing.hpp"

static void	parse_target_arguments(request_t& request)
{
	size_t pos = request.target.find('?', 0);	
	if (pos == std::string::npos)
		return ;
	request.query_string = request.target.substr(pos + 1, std::string::npos);
	std::string tmp = request.target.substr(0, pos);
	request.target = tmp;
}

static void	check_char_in_stream(const char& delimiter, std::istringstream& ss)
{
	char tmp;

	ss >> tmp;
	if (tmp != delimiter)
		throw std::invalid_argument("Unxpected token");
}

static void	parse_request_header(const std::string& header, request_t& request)
{
	/* Request format
	 * request-line = method SP request-target SP HTTP-version CRLF
	 */

	std::string			tmp;
	std::istringstream	ss(header);

	ss >> std::noskipws; /* To make whitespace non skipable */
	/* Parse Method */
	ss >> request.method;
	if (!(request.method == "GET" || request.method == "POST"
		|| request.method == "DELETE"))
		throw std::invalid_argument("Invalid Method" + request.method);

	check_char_in_stream(' ', ss);
	ss >> request.target;
	parse_target_arguments(request);
	check_char_in_stream(' ', ss);
	ss >> tmp;
	if (tmp != "HTTP/1.1")
		throw std::invalid_argument("expected HTTP/1.1 version");
	check_char_in_stream('\r', ss);
	check_char_in_stream('\n', ss);
	ss >> tmp;
	if (tmp != "Host:")
		throw std::invalid_argument("Expected host");
	check_char_in_stream(' ', ss);
	ss >> request.host;
	check_char_in_stream('\r', ss);
	check_char_in_stream('\n', ss);
	std::cout << GREEN "Before Cookie: " << ss.tellg() << RESET << std::endl;
	std::string::size_type pos = header.find("tracking-cookie=", ss.tellg());
	if (pos != std::string::npos) 
	{
		request.cookie = header.substr(pos + 16, 24);
	}
}

request_t*	get_parsed_request(const std::string& header)
{
	/* check request
	 * HOSTNAME:PORT
	 * PORT
	 * IP
	 * server_name
	*/
	request_t	*request = new request_t;
	parse_request_header(header, *request);
	return request;
}
