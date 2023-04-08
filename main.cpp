#include "parser.hpp"
#include "webserv.hpp"

ft::webserv webserver;

int main(int argc, char **argv)
{
	std::string filename("conf/webserv.conf");

	if (argc == 2)
		filename = argv[1];
	else if (argc != 1)
		return (webserver.error("Too many arguments!"));
	try
	{
		ft::parser config(filename);
		webserver.set_http(config.parse());
	}
	catch (const std::exception &e)
	{
		return (webserver.error(e.what()));
	}
}
