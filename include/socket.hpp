#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "server.hpp"

namespace ft
{
	class socket
	{
		private:
			int						_fd;
			server_pointer_vector	_servers;
		public:
			socket();
			~socket();
			socket(const socket &other);
			socket(int fd);
			socket &operator=(const socket &other);
			void add_server(const server *server);
			const server_pointer_vector &get_servers() const;
			int get_fd() const;
			operator int() const;
	};
}

#endif
