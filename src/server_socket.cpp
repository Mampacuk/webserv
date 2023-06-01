#include "server_socket.hpp"

namespace ft
{
	server_socket::~server_socket() {}

	server_socket::server_socket(const server_socket &other) : socket(other), _servers(other._servers) {}

	server_socket::server_socket(int fd, const std::string &host, const std::string &port) : socket(fd, host, port), _servers() {}

	server_socket &server_socket::operator=(const server_socket &other)
	{
		socket::operator=(other);
		_servers = other._servers;
		return (*this);
	}

	const server_pointer_vector &server_socket::get_servers() const
	{
		return (_servers);
	}

	void server_socket::add_server(const server *server)
	{
		_servers.push_back(server);
	}
}
