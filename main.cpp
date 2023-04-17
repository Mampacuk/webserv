#include "parser.hpp"
#include "webserv.hpp"
#include "request.hpp"

ft::webserv webserver;

// int main(int argc, char **argv)
int main()
{
	std::vector<std::string> request_chunks1 = {
		"GET /index.html HTTP/1.1\r\n",
		"Host: example.com\r\n",
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/89.0.4389.82 Safari/537.36\r\n",
		"Accept-Language: en-US,en;q=0.9\r\n",
		"Accept-Encoding: gzip, deflate, br\r\n",
		"Connection: keep-alive\r\n",
		"\r\n"
  	};
	ft::request req(1);
	for (size_t i = 0; i < request_chunks1.size(); i++)
	{
		std::cout << i << "completed? " << ((req += request_chunks1[i]) ? "yes" : "no") << std::endl;
	}
	req.parse();

	std::vector<std::string> request_chunks2 = {
    "GET /api/data HTTP/1.1\r\n",
    "Host: example.com\r\n",
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/89.0.4389.82 Safari/537.36\r\n",
    "Accept: */*\r\n",
    "Content-Type: application/json\r\n",
    "Content-Length: 56\r\n",
    "\r\n",
    "{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"}\r\n"
  };
	
	std::string request3 =
    "POST /submit HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 50\r\n"
    "\r\n"
    "{\"name\": \"John Doe\", \"age\": 30, \"email\": \"john.doe@example.com\"}\r\n";

	std::vector<std::string> request_chunks4 = {
    "POST /submit HTTP/1.1\r\n",
    "Host: example.com\r\n",
    "Content-Type: application/json\r\n",
    "Transfer-Encoding: chunked\r\n",
    "\r\n",
    "8\r\n",
    "{\"name\":\r\n",
    "c\r\n",
    "\"Alice\",\r\n",
    "a\r\n",
    "\"age\": 30\r\n",
    "0\r\n",
    "\r\n"
};
	// std::string filename("conf/webserv.conf");

	// if (argc == 2)
	// 	filename = argv[1];
	// else if (argc != 1)
	// 	return (webserver.error("Too many arguments!"));
	// try
	// {
	// 	ft::parser config(filename);
	// 	webserver.set_http(config.parse());
	// }
	// catch (const std::exception &e)
	// {
	// 	return (webserver.error(e.what()));
	// }
}

// dummy definition for Windows
# if defined(_WIN32) || defined(__CYGWIN__)
	int fcntl(int, int, ...) { return (0); }
#endif