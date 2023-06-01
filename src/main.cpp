#include "parser.hpp"

int main(int argc, char **argv)
{
	ft::webserv webserver;
	std::string filename("conf/webserv.conf");

	if (argc == 2)
		filename = argv[1];
	else if (argc != 1)
		return (ft::webserv::error("Invalid arguments. Usage: ./webserv [config file]"));
	try
	{
		ft::parser config(filename);
		webserver.set_http(config.parse());
		webserver.serve();
		return (EXIT_SUCCESS);
	}
	catch (const std::exception &e)
	{
		return (ft::webserv::error(e.what()));
	}
}
