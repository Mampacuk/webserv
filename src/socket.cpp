#include "socket.hpp"

namespace ft
{
	socket::socket() : _fd(), _servers() {}

	socket::socket(const socket &other) : _fd(other._fd), _servers(other._servers) {}

	socket::socket(int fd) : _fd(fd), _servers() {}

	socket::~socket() {}

	socket &socket::operator=(const socket &other)
	{
		this->_fd = other._fd;
		this->_servers = other._servers;
		return (*this);
	}

	void socket::add_server(const server *server)
	{
		this->_servers.push_back(server);
	}

	const server_pointer_vector &socket::get_servers() const
	{
		return (this->_servers);
	}

	int socket::get_fd() const
	{
		return (this->_fd);
	}

	socket::operator int() const
	{
		return (this->_fd);
	}
}
