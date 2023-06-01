#include "socket.hpp"

namespace ft
{
	socket::~socket() {}

	socket::socket(const socket &other) : _fd(other._fd), _host(other._host), _port(other._port) {}

	socket::socket(int fd, const std::string &host, const std::string &port) : _fd(fd), _host(host), _port(port) {}

	socket &socket::operator=(const socket&) { return (*this); }

	int socket::get_fd() const
	{
		return (_fd);
	}

	socket::operator int() const
	{
		return (_fd);
	}

	const std::string &socket::get_host() const
	{
		return (_host);
	}

	const std::string &socket::get_port() const
	{
		return (_port);
	}

}
