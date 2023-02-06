#include "parser.hpp"
#include "webserv.hpp"

ft::webserv webserv;

int main(int argc, char **argv)
{
	std::string filename("conf/webserv.conf");

	if (argc == 2)
		filename = argv[1];
	else if (argc != 1)
		return (webserv.error("Too many arguments!"));
	try
	{
		ft::parser config(filename); // check exception in ctor scenario for memory leaks
		webserv.set_http(config.parse());
	}
	catch (const std::exception &e)
	{
		return (webserv.error(e.what()));
	}
	webserv.verify_http();
}
