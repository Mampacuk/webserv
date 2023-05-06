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
   pid_t waitpid(...);
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
# include <signal.h>

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
	class http;
	class webserv;
	class server;
	class location;
	class request;
	class response;
	class socket;
	class server_socket;

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
	std::string get_file_extension(const std::string &filename);

	std::string inet_ntoa(struct in_addr addr);

	template <typename Integral>
	std::string to_string(Integral val)
	{
		std::stringstream ss;
		ss << val;
		return (ss.str());
	}

	class parsing_error : public std::logic_error
	{
		public:
			explicit parsing_error(const std::string &what) : std::logic_error(what) {}
			explicit parsing_error(const char *what) : std::logic_error(what) {}
	};

	unsigned int strtoul(const std::string &number, int base = 10)
	{
		if (number.empty() || number[0] == '-' || std::isspace(number[0]) || base < 2)
			throw parsing_error("Number parsing error occured.");
		const char *str_begin = number.c_str();
		char *str_end = NULL;
		unsigned long long int ul = std::strtoul(str_begin, &str_end, base);
		if (str_end != str_begin + number.length() || ul > std::numeric_limits<unsigned int>::max())
			throw parsing_error("Number parsing error occured.");
		return (ul);
	}

	enum http_code
	{
		/*####### 1xx - Informational #######*/
		/* Indicates an interim response for communicating connection status
		* or request progress prior to completing the requested action and
		* sending a final response.
		*/
		continue_transfer   = 100, //!< Indicates that the initial part of a request has been received and has not yet been rejected by the server.
		switching_protocols = 101, //!< Indicates that the server understands and is willing to comply with the client's request, via the Upgrade header field, for a change in the application protocol being used on this connection.

		/*####### 2xx - Successful #######*/
		/* Indicates that the client's request was successfully received,
		* understood, and accepted.
		*/
		ok                            = 200, //!< Indicates that the request has succeeded.
		created                       = 201, //!< Indicates that the request has been fulfilled and has resulted in one or more new resources being created.
		accepted                      = 202, //!< Indicates that the request has been accepted for processing, but the processing has not been completed.
		non_authoritative_information = 203, //!< Indicates that the request was successful but the enclosed payload has been modified from that of the origin server's 200 (OK) response by a transforming proxy.
		no_content                    = 204, //!< Indicates that the server has successfully fulfilled the request and that there is no additional content to send in the response payload body.
		reset_content                 = 205, //!< Indicates that the server has fulfilled the request and desires that the user agent reset the \"document view\", which caused the request to be sent, to its original state as received from the origin server.
		partial_content               = 206, //!< Indicates that the server is successfully fulfilling a range request for the target resource by transferring one or more parts of the selected representation that correspond to the satisfiable ranges found in the requests's Range header field.

		/*####### 3xx - Redirection #######*/
		/* Indicates that further action needs to be taken by the user agent
		* in order to fulfill the request.
		*/
		multiple_choices   = 300, //!< Indicates that the target resource has more than one representation, each with its own more specific identifier, and information about the alternatives is being provided so that the user (or user agent) can select a preferred representation by redirecting its request to one or more of those identifiers.
		moved_permanently  = 301, //!< Indicates that the target resource has been assigned a new permanent URI and any future references to this resource ought to use one of the enclosed URIs.
		found              = 302, //!< Indicates that the target resource resides temporarily under a different URI.
		see_other          = 303, //!< Indicates that the server is redirecting the user agent to a different resource, as indicated by a URI in the Location header field, that is intended to provide an indirect response to the original request.
		not_modified       = 304, //!< Indicates that a conditional GET request has been received and would have resulted in a 200 (OK) response if it were not for the fact that the condition has evaluated to false.
		use_proxy          = 305, //!< \deprecated \parblock Due to security concerns regarding in-band configuration of a proxy. \endparblock
									//!< The requested resource MUST be accessed through the proxy given by the Location field.
		temporary_redirect = 307, //!< Indicates that the target resource resides temporarily under a different URI and the user agent MUST NOT change the request method if it performs an automatic redirection to that URI.
		permanent_redirect = 308, //!< The target resource has been assigned a new permanent URI and any future references to this resource outght to use one of the enclosed URIs. [...] This status code is similar to 301 Moved Permanently (Section 7.3.2 of rfc7231), except that it does not allow rewriting the request method from POST to GET.

		/*####### 4xx - Client Error #######*/
		/* Indicates that the client seems to have erred.
		*/
		bad_request                   = 400, //!< Indicates that the server cannot or will not process the request because the received syntax is invalid, nonsensical, or exceeds some limitation on what the server is willing to process.
		unauthorized                  = 401, //!< Indicates that the request has not been applied because it lacks valid authentication credentials for the target resource.
		payment_required              = 402, //!< *Reserved*
		forbidden                     = 403, //!< Indicates that the server understood the request but refuses to authorize it.
		not_found                     = 404, //!< Indicates that the origin server did not find a current representation for the target resource or is not willing to disclose that one exists.
		method_not_allowed            = 405, //!< Indicates that the method specified in the request-line is known by the origin server but not supported by the target resource.
		not_acceptable                = 406, //!< Indicates that the target resource does not have a current representation that would be acceptable to the user agent, according to the proactive negotiation header fields received in the request, and the server is unwilling to supply a default representation.
		proxy_authentication_required = 407, //!< Is similar to 401 (Unauthorized), but indicates that the client needs to authenticate itself in order to use a proxy.
		request_timeout               = 408, //!< Indicates that the server did not receive a complete request message within the time that it was prepared to wait.
		conflict                      = 409, //!< Indicates that the request could not be completed due to a conflict with the current state of the resource.
		gone                          = 410, //!< Indicates that access to the target resource is no longer available at the origin server and that this condition is likely to be permanent.
		length_required               = 411, //!< Indicates that the server refuses to accept the request without a defined Content-Length.
		precondition_failed           = 412, //!< Indicates that one or more preconditions given in the request header fields evaluated to false when tested on the server.
		content_too_large             = 413, //!< Indicates that the server is refusing to process a request because the request payload is larger than the server is willing or able to process.
		payload_too_large             = 413, //!< Alias for ContentTooLarge for backward compatibility.
		uri_too_long                  = 414, //!< Indicates that the server is refusing to service the request because the request-target is longer than the server is willing to interpret.
		unsupported_media_type        = 415, //!< Indicates that the origin server is refusing to service the request because the payload is in a format not supported by the target resource for this method.
		range_not_satisfiable         = 416, //!< Indicates that none of the ranges in the request's Range header field overlap the current extent of the selected resource or that the set of ranges requested has been rejected due to invalid ranges or an excessive request of small or overlapping ranges.
		expectation_failed            = 417, //!< Indicates that the expectation given in the request's Expect header field could not be met by at least one of the inbound servers.

		/*####### 5xx - Server Error #######*/
		/* Indicates that the server is aware that it has erred
		* or is incapable of performing the requested method.
		*/
		internal_server_error      = 500, //!< Indicates that the server encountered an unexpected condition that prevented it from fulfilling the request.
		not_implemented            = 501, //!< Indicates that the server does not support the functionality required to fulfill the request.
		bad_gateway                = 502, //!< Indicates that the server, while acting as a gateway or proxy, received an invalid response from an inbound server it accessed while attempting to fulfill the request.
		service_unavailable        = 503, //!< Indicates that the server is currently unable to handle the request due to a temporary overload or scheduled maintenance, which will likely be alleviated after some delay.
		gateway_timeout            = 504, //!< Indicates that the server, while acting as a gateway or proxy, did not receive a timely response from an upstream server it needed to access in order to complete the request.
		http_version_not_supported = 505, //!< Indicates that the server does not support, or refuses to support, the protocol version that was used in the request message.

		xxx_max = 1023
	};

	std::string reason_phrase(http_code status)
	{
		switch (status)
		{
			//####### 1xx - Informational #######
			case continue_transfer:		return ("Continue");
			case switching_protocols:	return ("Switching Protocols");

			//####### 2xx - Successful #######
			case ok:							return ("OK");
			case created:						return ("Created");
			case accepted:						return ("Accepted");
			case non_authoritative_information:	return ("Non-Authoritative Information");
			case no_content:					return ("No Content");
			case reset_content:					return ("Reset Content");
			case partial_content:				return ("Partial Content");

			//####### 3xx - Redirection #######
			case multiple_choices:		return ("Multiple Choices");
			case moved_permanently:		return ("Moved Permanently");
			case found:					return ("Found");
			case see_other:				return ("See Other");
			case not_modified:			return ("Not Modified");
			case use_proxy:				return ("Use Proxy");
			case temporary_redirect:	return ("Temporary Redirect");
			case permanent_redirect:	return ("Permanent Redirect");

			//####### 4xx - Client Error #######
			case bad_request:					return ("Bad Request");
			case unauthorized:					return ("Unauthorized");
			case payment_required:				return ("Payment Required");
			case forbidden:						return ("Forbidden");
			case not_found:						return ("Not Found");
			case method_not_allowed:			return ("Method Not Allowed");
			case not_acceptable:				return ("Not Acceptable");
			case proxy_authentication_required:	return ("Proxy Authentication Required");
			case request_timeout:				return ("Request Timeout");
			case conflict:						return ("Conflict");
			case gone:							return ("Gone");
			case length_required:				return ("Length Required");
			case precondition_failed:			return ("Precondition Failed");
			case content_too_large:				return ("Content Too Large");
			case uri_too_long:					return ("URI Too Long");
			case unsupported_media_type:		return ("Unsupported Media Type");
			case range_not_satisfiable:			return ("Range Not Satisfiable");
			case expectation_failed:			return ("Expectation Failed");

			//####### 5xx - Server Error #######
			case internal_server_error:			return ("Internal Server Error");
			case not_implemented:				return ("Not Implemented");
			case bad_gateway:					return ("Bad Gateway");
			case service_unavailable:			return ("Service Unavailable");
			case gateway_timeout:				return ("Gateway Timeout");
			case http_version_not_supported:	return ("HTTP Version Not Supported");

			default: return (std::string());
		}
	}

	class protocol_error : public std::logic_error
	{
		private:
			http_code _error;
		public:
			protocol_error(http_code error, const std::string &what) : std::logic_error(what), _error(error) {}
			protocol_error(http_code error, const char *what) : std::logic_error(what), _error(error) {}
			operator http_code() const { return (this->_error); }
	};
}

#endif
