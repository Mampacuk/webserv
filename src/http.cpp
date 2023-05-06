#include "http.hpp"

namespace ft
{
	http::http(): base_dir(), _sockets(), _servers() {}

	http::~http() {}

	http::http(const http &other): base_dir(other), _sockets(other._sockets), _servers(other._servers) {}

	http &http::operator=(const http &other)
	{
		base_dir::operator=(other);
		this->_sockets = other._sockets;
		this->_servers = other._servers;
		return (*this);
	}

	const server_list &http::get_servers() const
	{
		return (this->_servers);
	}

	const server_socket_set &http::get_sockets() const
	{
		return (this->_sockets);
	}

	void http::add_server(server server)
	{
		this->_servers.push_back(server);
	}

	void http::add_socket(const socket &socket)
	{
		this->_sockets.insert(socket);
	}

	void http::close_sockets()
	{
		for (server_socket_set::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
			close(*it);
	}

	bool http::is_port_number(const std::string &port_string)
	{
		try
		{
			const unsigned int port = parser::strtoul(port_string);
			return (port >= 1 && port <= 65535);
		}
		catch (const std::exception &e)
		{
			return (false);
		}
	}

	bool http::is_informational_code(http_code status)
	{
		return (status >= 100 && status < 200);
	}

	bool http::is_successful_code(http_code status)
	{
		return (status >= 200 && status < 300);
	}

	bool http::is_redirection_code(http_code status)
	{
		return (status >= 300 && status < 400);
	}

	bool http::is_client_error_code(http_code status)
	{
		return (status >= 400 && status < 500);
	}

	bool http::is_server_error_code(http_code status)
	{
		return (status >= 500 && status < 600);
	}
	
	bool http::is_error_code(http_code status)
	{
		return (status >= 400);
	}

	std::string http::reason_phrase(http_code status)
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
}
