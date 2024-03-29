#include "stdafx.hpp"

namespace ft
{
	case_insensitive_equal_to::result_type case_insensitive_equal_to::operator()(const case_insensitive_equal_to::first_argument_type &lhs, const case_insensitive_equal_to::second_argument_type &rhs)
	{
		return (std::tolower(lhs) == std::tolower(rhs));
	}

    bool ends_with(const std::string &str, const std::string &suffix)
	{
		return (str.size() >= suffix.size() && !str.compare(str.size() - suffix.size(), std::string::npos, suffix));
	}

	bool starts_with(const std::string &str, const std::string &prefix)
	{
		return (str.size() >= prefix.size() && !str.compare(0, prefix.size(), prefix));
	}

	std::string get_file_extension(const std::string &filename)
	{
		std::string extension;
		const size_t dot_pos = filename.find_last_of(".");
		if (dot_pos != std::string::npos && dot_pos < filename.size() - 1)
			extension = filename.substr(dot_pos + 1);
		return (extension);
	}

	std::string inet_ntoa(struct in_addr addr)
	{
		std::stringstream ss;
		ss << ((addr.s_addr >> 0) & 0xff) << '.'
		   << ((addr.s_addr >> 8) & 0xff) << '.'
		   << ((addr.s_addr >> 16) & 0xff) << '.'
		   << ((addr.s_addr >> 24) & 0xff);
		return (ss.str());
	}

	// doesn't tolerate leading or trailing whitespaces,
	// or characters in the middle of the number.
	unsigned int strtoul(const std::string &number, int base)
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

	char toupper(char c)
	{
		return (std::toupper(c));
	}

	char tounderscore(char c)
	{
		if (c == '-')
			return ('_');
		return (c);
	}

	std::string uppercase(const std::string &str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), toupper);
		return (result);
	}

	std::string underscore(const std::string &str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), tounderscore);
		return (result);
	}

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
			case multi_status:					return "Multi-Status";
			case already_reported:				return "Already Reported";
			case im_used:						return "IM Used";

			//####### 3xx - Redirection #######
			case multiple_choices:		return ("Multiple Choices");
			case moved_permanently:		return ("Moved Permanently");
			case found:					return ("Found");
			case see_other:				return ("See Other");
			case not_modified:			return ("Not Modified");
			case use_proxy:				return ("Use Proxy");
			case temporary_rewrite:		return ("Temporary Redirect");
			case permanent_rewrite:		return ("Permanent Redirect");

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
}
