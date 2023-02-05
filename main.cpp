#include "parser.hpp"
#include "webserv.hpp"

ft::webserv webserv;

int main(int argc, char **argv)
{
	{
		std::string filename("conf/webserv.conf");

		if (argc == 2)
			filename = argv[1];
		else if (argc != 1)
			throw std::invalid_argument("Too many arguments!");
		ft::parser config(filename);
		webserv.set_http(static_cast<ft::http*>(config.parse()));
		webserv.verify_http();
	}
}
