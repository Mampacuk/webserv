#ifndef SERVER_SOCKET_HPP
# define SERVER_SOCKET_HPP

// # include "server.hpp" //instread of this inlcude socket.hpp
# include "socket.hpp"

namespace ft
{
	class server_socket : public socket
	{
		private:
            server_pointer_vector	_servers;
			server_socket();
		public:
			~server_socket();
			server_socket(const server_socket &other);
			server_socket(int fd, const std::string &host, const std::string &port);
			server_socket &operator=(const server_socket &other);
			void add_server(const server *server);
			const server_pointer_vector &get_servers() const;
	};
}

#endif
