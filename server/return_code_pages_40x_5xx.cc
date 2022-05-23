#include "./includes/webserver.hpp"

void initialize_html_return_code_page(t_return_codes *return_codes)
{
	return_codes->err_400 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>400 Bad Request</title>\n</head>\n</html>";
	return_codes->err_401 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>401 Unauthorized</title>\n</head>\n</html>";
	return_codes->err_403 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>403 Forbidden</title>\n</head>\n</html>";
	return_codes->err_404 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>404 Not Found</title>\n</head>\n</html>";
	return_codes->err_405 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>405 Method Not Allowed</title>\n</head>\n</html>";
	return_codes->err_406 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>406 Not Acceptable</title>\n</head>\n</html>";
	return_codes->err_408 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>408 Request Timeout</title>\n</head>\n</html>";
	return_codes->err_413 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>413 Payload Too Large</title>\n</head>\n</html>";
	return_codes->err_414 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>414 URI Too Long</title>\n</head>\n</html>";
	return_codes->err_429 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>429 Too Many Requests</title>\n</head>\n</html>";
	return_codes->err_431 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>431 Request Header Fields Too Large</title>\n</head>\n</html>";
	return_codes->err_500 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>500 Internal Server Error</title>\n</head>\n</html>";
	return_codes->err_501 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>501 Not Implemented</title>\n</head>\n</html>";
	return_codes->err_502 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>502 Bad Gateway</title>\n</head>\n</html>";
	return_codes->err_503 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>503 Service Unavailable</title>\n</head>\n</html>";
	return_codes->err_504 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>504 Gateway Timeout</title>\n</head>\n</html>";
	return_codes->err_505 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>505 HTTP Version Not Supported</title>\n</head>\n</html>";
	return_codes->err_511 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>511 Network Authentication Required</title>\n</head>\n</html>";
}

void	change_default_html_return_code(std::string path, std::string *return_code)
{
	if (access(path.c_str(), F_OK) == -1 || access(path.c_str(), R_OK) == -1)
		std::cerr << "Error: " << path << " does not exist or is not readable. Default page kept." << std::endl;
	else
	{
		std::ifstream file(path.c_str());
		std::string file_str;
		if (file)
		{
			std::stringstream str_stream;
			str_stream << file.rdbuf();
			file_str = str_stream.str();
		}
		else
			throw std::invalid_argument("Ifstream failed to open file " + path);
		return_code->clear();
		return_code = &file_str;
	}
}

template <typename T>
void	set_default_return_code(T & datas)
{
	std::vector<uint16_str_pair>::iterator it;
	std::vector<uint16_str_pair>::iterator it_end;
	std::string root_path;
	t_return_codes *return_codes;

	it = datas->get_error_pages().begin();
	it_end = datas->get_error_pages().end();
	return_codes = &datas->return_codes;
	initialize_html_return_code_page(&(*return_codes));
	root_path = datas->get_root_path();
	for (; it != it_end; ++it)
	{
		switch (it->first)
		{
			case 400:
				change_default_html_return_code(root_path + it->second, &return_codes->err_400);
				break;
			case 401:
				change_default_html_return_code(root_path + it->second, &return_codes->err_401);
				break;
			case 403:
				change_default_html_return_code(root_path + it->second, &return_codes->err_403);
				break;
			case 404:
				change_default_html_return_code(root_path + it->second, &return_codes->err_404);
				break;
			case 405:
				change_default_html_return_code(root_path + it->second, &return_codes->err_405);
				break;
			case 406:
				change_default_html_return_code(root_path + it->second, &return_codes->err_406);
				break;
			case 408:
				change_default_html_return_code(root_path + it->second, &return_codes->err_408);
				break;
			case 413:
				change_default_html_return_code(root_path + it->second, &return_codes->err_413);
				break;
			case 414:
				change_default_html_return_code(root_path + it->second, &return_codes->err_414);
				break;
			case 429:
				change_default_html_return_code(root_path + it->second, &return_codes->err_429);
				break;
			case 431:
				change_default_html_return_code(root_path + it->second, &return_codes->err_431);
				break;
			case 500:
				change_default_html_return_code(root_path + it->second, &return_codes->err_500);
				break;
			case 501:
				change_default_html_return_code(root_path + it->second, &return_codes->err_501);
				break;
			case 502:
				change_default_html_return_code(root_path + it->second, &return_codes->err_502);
				break;
			case 503:
				change_default_html_return_code(root_path + it->second, &return_codes->err_503);
				break;
			case 504:
				change_default_html_return_code(root_path + it->second, &return_codes->err_504);
				break;
			case 505:
				change_default_html_return_code(root_path + it->second, &return_codes->err_505);
				break;
			case 511:
				change_default_html_return_code(root_path + it->second, &return_codes->err_511);
				break;
			default:
				break;
		}
	}
}
