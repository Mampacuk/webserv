#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "server.hpp"

namespace ft
{
	class socket
	{
		private:
			int				_fd;
			const server	&_server;
			socket();
			socket operator=(const socket &other);
		public:
			socket(const socket &other);
			socket(int fd, const server &server);
			~socket();
			const server &get_server() const;
			operator int() const;
			
	};
}

#endif