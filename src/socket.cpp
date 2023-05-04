#include "socket.hpp"

namespace ft
{
	socket::socket() : _fd() {}

	socket::socket(const socket &other) : _fd(other._fd) {}

	socket::socket(int fd, const std::string &host, const std::string &port) : _fd(fd), _host(host), _port(port) {}

	socket::~socket() {}

	socket &socket::operator=(const socket &other) { return (*this); }

	int socket::get_fd() const
	{
		return (this->_fd);
	}

	socket::operator int() const
	{
		return (this->_fd);
	}
}
