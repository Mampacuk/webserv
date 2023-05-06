#include "client_socket.hpp"

namespace ft
{
	client_socket::client_socket(const client_socket &other) : ft::socket(other), _socket(other._socket) {}

	client_socket::client_socket(int fd, const std::string &host, const std::string &port,  const server_socket &socket) : ft::socket(fd, host, port), _socket(socket) {}

	client_socket::~client_socket() {}

	client_socket &client_socket::operator=(const client_socket &other)
	{
		socket::operator=(other);
		return (*this);
	}

    const server_socket &client_socket::get_server_socket() const
    {
        return (_socket);
    }
}