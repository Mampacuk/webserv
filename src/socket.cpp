#include "socket.hpp"

namespace ft
{
    socket::socket(const socket &other) : _fd(other._fd), _server(other._server) {}

    socket::socket(int fd, const server &server) : _fd(fd), _server(server) {}

    socket::~socket() {}

    const server &socket::get_server() const
    {
        return (this->_server);
    }

    socket::operator int() const
    {
        return (this->_fd);
    }
}
