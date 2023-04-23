#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "server.hpp"

namespace ft
{
	class socket
	{
		private:
			int		_socket_fd;
			server	&_server;
			
			socket();
			socket operator=(const socket &other);
		public:
			socket(const socket &other);
			socket(int socket_fd, server &server);
			~socket();
			server &get_server() const;
			operator int() const;
			
	};
}

#endif