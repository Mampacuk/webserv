#ifndef STDAFX_HPP
# define STDAFX_HPP

# if defined(_WIN32) || defined(__CYGWIN__) // defined on windows
#  undef UNICODE
#  define NOMINMAX
#  define WINVER 0x0A00
#  define _WIN32_WINNT 0x0A00
#  define SO_REUSEPORT 0x0200
#  include <WinSock2.h>
#  include <WS2tcpip.h>
#  include <wspiapi.h>
#  include <io.h>
#  include <winhttp.h>
#  define F_SETFL 4
#  define O_NONBLOCK 00004000
   int fcntl(int, int, ...);
# elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/tcp.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <fcntl.h>

//
// HTTP Response Status Codes:
//

#  define HTTP_STATUS_CONTINUE				100 // OK to continue with request
#  define HTTP_STATUS_SWITCH_PROTOCOLS		101 // server has switched protocols in upgrade header

#  define HTTP_STATUS_OK					200 // request completed
#  define HTTP_STATUS_CREATED				201 // object created, reason = new URI
#  define HTTP_STATUS_ACCEPTED				202 // async completion (TBS)
#  define HTTP_STATUS_PARTIAL				203 // partial completion
#  define HTTP_STATUS_NO_CONTENT			204 // no info to return
#  define HTTP_STATUS_RESET_CONTENT			205 // request completed, but clear form
#  define HTTP_STATUS_PARTIAL_CONTENT		206 // partial GET fulfilled
#  define HTTP_STATUS_WEBDAV_MULTI_STATUS	207 // WebDAV Multi-Status

#  define HTTP_STATUS_AMBIGUOUS				300 // server couldn't decide what to return
#  define HTTP_STATUS_MOVED					301 // object permanently moved
#  define HTTP_STATUS_REDIRECT				302 // object temporarily moved
#  define HTTP_STATUS_REDIRECT_METHOD		303 // redirection w/ new access method
#  define HTTP_STATUS_NOT_MODIFIED			304 // if-modified-since was not modified
#  define HTTP_STATUS_USE_PROXY				305 // redirection to proxy, location header specifies proxy to use
#  define HTTP_STATUS_REDIRECT_KEEP_VERB	307 // HTTP/1.1: keep same verb
#  define HTTP_STATUS_PERMANENT_REDIRECT	308 // Object permanently moved keep verb

#  define HTTP_STATUS_BAD_REQUEST			400 // invalid syntax
#  define HTTP_STATUS_DENIED				401 // access denied
#  define HTTP_STATUS_PAYMENT_REQ			402 // payment required
#  define HTTP_STATUS_FORBIDDEN				403 // request forbidden
#  define HTTP_STATUS_NOT_FOUND				404 // object not found
#  define HTTP_STATUS_BAD_METHOD			405 // method is not allowed
#  define HTTP_STATUS_NONE_ACCEPTABLE		406 // no response acceptable to client found
#  define HTTP_STATUS_PROXY_AUTH_REQ		407 // proxy authentication required
#  define HTTP_STATUS_REQUEST_TIMEOUT		408 // server timed out waiting for request
#  define HTTP_STATUS_CONFLICT				409 // user should resubmit with more info
#  define HTTP_STATUS_GONE					410 // the resource is no longer available
#  define HTTP_STATUS_LENGTH_REQUIRED		411 // the server refused to accept request w/o a length
#  define HTTP_STATUS_PRECOND_FAILED		412 // precondition given in request failed
#  define HTTP_STATUS_REQUEST_TOO_LARGE		413 // request entity was too large
#  define HTTP_STATUS_URI_TOO_LONG			414 // request URI too long
#  define HTTP_STATUS_UNSUPPORTED_MEDIA		415 // unsupported media type
#  define HTTP_STATUS_RETRY_WITH			449 // retry after doing the appropriate action.

#  define HTTP_STATUS_SERVER_ERROR			500 // internal server error
#  define HTTP_STATUS_NOT_SUPPORTED			501 // required not supported
#  define HTTP_STATUS_BAD_GATEWAY			502 // error response received from gateway
#  define HTTP_STATUS_SERVICE_UNAVAIL		503 // temporarily overloaded
#  define HTTP_STATUS_GATEWAY_TIMEOUT		504 // timed out waiting for gateway
#  define HTTP_STATUS_VERSION_NOT_SUP		505 // HTTP version not supported

# endif

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

# define RED	"\033[31m"
# define GREEN	"\033[32m"
# define YELLOW	"\033[33m"
# define RESET	"\033[0m"
# define EL		"\033[2K" // erase line
# define CRLF	"\r\n"

# define BACKLOG			128
# define TIMEOUT_SEC		0
# define TIMEOUT_MICROSEC	500000

namespace ft
{
	class server;
	class location;
	class request;
	class response;

	typedef std::pair<int, std::string>				int_string;
	typedef std::pair<std::string, std::string>		string_pair;
	typedef std::pair<unsigned int, std::string>	error_page;
	
	typedef std::map<int, std::string>			int_string_map;
	typedef std::map<std::string, std::string>	string_map;
	typedef std::map<unsigned int, std::string>	error_map;

	typedef std::set<int>			int_set;
	typedef std::set<string_pair>	string_pair_set;
	typedef std::set<std::string>	string_set;
	
	typedef std::multimap<std::string, std::string> string_mmap;
	
	typedef std::vector<int>			int_vector;
	typedef std::vector<std::string>	string_vector;
	typedef std::vector<server>			server_vector;
	
	typedef std::list<std::string>	string_list;
	typedef std::list<request>		request_list;
	typedef std::list<response>		response_list;

	typedef std::set<location> location_set;

	// defined in parser.cpp
	bool ends_with(const std::string &str, const std::string &suffix);
}

#endif
