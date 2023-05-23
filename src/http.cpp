#include "http.hpp"

namespace ft
{
	http::http(): base_dir(), _sockets(), _servers() {}

	http::~http() {}

	http::http(const http &other): base_dir(other), _sockets(other._sockets), _servers(other._servers) {}

	http &http::operator=(const http &other)
	{
		base_dir::operator=(other);
		_sockets = other._sockets;
		_servers = other._servers;
		return (*this);
	}

	const server_list &http::get_servers() const
	{
		return (_servers);
	}

	const server_socket_set &http::get_sockets() const
	{
		return (_sockets);
	}

	void http::add_server(server server)
	{
		_servers.push_back(server);
	}

	void http::add_socket(const server_socket &socket)
	{
		_sockets.insert(socket);
	}

	void http::close_sockets()
	{
		for (server_socket_set::iterator it = _sockets.begin(); it != _sockets.end(); it++)
			close(*it);
	}

	bool http::is_port_number(const std::string &port_string)
	{
		try
		{
			const unsigned int port = ft::strtoul(port_string);
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
}
