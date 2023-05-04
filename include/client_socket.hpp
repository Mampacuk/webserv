#ifndef CLIENT_SOCKET_HPP
# define CLIENT_SOCKET_HPP

# include "server_socket.hpp"

namespace ft
{
    class client_socket : public socket
    {
        private:
            const server_socket &_socket;
		public:
			client_socket();
			~client_socket();
			client_socket(const client_socket &other);
			client_socket(int fd, const std::string &host, const std::string &port, const server_socket &socket);
			client_socket &operator=(const client_socket &other);
            const server_socket &get_server_socket() const;
    };
}

#endif