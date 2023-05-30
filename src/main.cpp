#include "parser.hpp"

static void print_leaks()
{
	system("leaks webserv");
}

int main(int argc, char **argv)
{
	atexit(print_leaks);
	ft::webserv webserver;
	std::string filename("conf/webserv.conf");

	if (argc == 2)
		filename = argv[1];
	else if (argc != 1)
		return (ft::webserv::error("Too many arguments!"));
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

// dummy definition for Windows
# if defined(_WIN32) || defined(__CYGWIN__)
	int fcntl(int, int, ...) { return (0); }
#endif
