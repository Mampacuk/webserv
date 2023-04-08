#ifndef STDAFX_HPP
# define STDAFX_HPP

// # if defined(_WIN32) || defined(__CYGWIN__) // defined on windows
// #  undef UNICODE
// #  define NOMINMAX
// #  define WINVER 0x0A00
// #  define _WIN32_WINNT 0x0A00
// #  include <WinSock2.h>
// #  include <WS2tcpip.h>
// #  include <wspiapi.h>
// #  include <io.h>
// # elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/tcp.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <fcntl.h>
// # endif

# include <string>
# include <cstring>
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

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RESET "\033[0m"

# define BACKLOG 128
# define CLRF "\r\n"

namespace ft
{
	class server;
	class location;

	typedef std::map<std::string, std::string> string_map;
	typedef std::set<std::string> string_set;
	typedef std::multimap<std::string, std::string> string_mmap;
	typedef std::vector<std::string> string_vector;
	typedef std::list<std::string> string_list;
	typedef std::pair<std::string, std::string> string_pair;
	typedef std::set<string_pair> string_pair_set;
	typedef std::map<unsigned int, std::string> error_map;
	typedef std::pair<unsigned int, std::string> error_page;
	typedef std::vector<int> int_vector;
	typedef std::set<int> int_set;
	typedef std::vector<server> server_vector;
	typedef std::set<location> location_set;
	typedef std::map<int, std::string> int_string_map;
	typedef std::pair<int, std::string> int_string;
}

#endif
