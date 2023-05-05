#ifndef STDAFX_HPP
# define STDAFX_HPP

# if defined(_WIN32) || defined(__CYGWIN__) // defined on windows
#  undef UNICODE
#  define NOMINMAX
#  define WINVER 0x0A00
#  define _WIN32_WINNT 0x0A00
#  include <WinSock2.h>
#  include <WS2tcpip.h>
#  include <wspiapi.h>
#  include <io.h>
#  define SO_REUSEPORT 0x0200
#  define F_SETFL 4
#  define O_NONBLOCK 00004000
#  ifdef _WIN64
#   define ssize_t __int64
#  else
#   define ssize_t long
#  endif
#  define STDIN_FILENO 0
#  define STDOUT_FILENO 1
#  define STDERR_FILENO 2
   typedef int pid_t;
   int fcntl(int, int, ...);
   int kill(...);
   pid_t fork(...);
   int pipe(...);
   int execve(...);
   typedef struct {} DIR;
   DIR *opendir(...);
   typedef struct { char *d_name; } dirent;
   dirent *readdir(...);
   void closedir(...);
   pid_t wait_pid(...);
#  define WTERMSIG(status)    ((status) & 0x7f)
#  define WIFEXITED(status)   (WTERMSIG(status) == 0)
#  define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#  define SIGTERM 15
# elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/tcp.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <dirent.h>
# endif

# include <string>
# include <cstring>
# include <string.h>
# include <cstdlib>
# include <fstream>
# include <sstream>
# include <limits>
# include <stdexcept>
# include <iostream>
# include <algorithm>
# include <list>
# include <set>
# include <map> 
# include <vector>

# define RED	"\033[31m"
# define GREEN	"\033[32m"
# define YELLOW	"\033[33m"
# define RESET	"\033[0m"
# define EL		"\033[2K" // erase line
# define CRLF	"\r\n"

# define HTTP_VERSION		"HTTP/1.1"
# define BACKLOG			128
# define TIMEOUT_SEC		0
# define TIMEOUT_MICROSEC	500000

namespace ft
{
	class server;
	class location;
	class request;
	class response;
	class socket;

	typedef std::pair<int, std::string>				int_string;
	typedef std::pair<std::string, std::string>		string_pair;
	typedef std::pair<unsigned int, std::string>	error_page;
	
	typedef std::map<int, std::string>			int_string_map;
	typedef std::map<std::string, std::string>	string_map;
	typedef std::map<unsigned int, std::string>	error_map;

	typedef std::set<int>			int_set;
	typedef std::set<socket>		socket_set;
	typedef std::set<server_socket>	server_socket_set;
	typedef std::set<string_pair>	string_pair_set;
	typedef std::set<std::string>	string_set;
	
	typedef std::multimap<std::string, std::string> string_mmap;
	typedef std::multimap<string_pair, const server*> string_pair_server_pointer_mmap;
	
	typedef std::vector<int>			int_vector;
	typedef std::vector<socket>			socket_vector;
	typedef std::vector<std::string>	string_vector;
	typedef std::vector<const server*>	server_pointer_vector;
	
	typedef std::list<std::string>	string_list;
	typedef std::list<request>		request_list;
	typedef std::list<response>		response_list;
	typedef std::list<server>		server_list;

	typedef std::set<location> location_set;

	// defined in parser.cpp
	bool ends_with(const std::string &str, const std::string &suffix);
	bool starts_with(const std::string &str, const std::string &prefix);
	std::string inet_ntoa(struct in_addr addr);

	template <typename Integral>
	std::string to_string(Integral val)
	{
		std::stringstream ss;
		ss << val;
		return (ss.str());
	}
}

#endif
