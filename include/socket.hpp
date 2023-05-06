#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "server.hpp"

namespace ft
{
	class socket
	{
		private:
			const int	_fd;
			const std::string _host;
			const std::string _port;
		public:
			socket();
			~socket();
			socket(const socket &other);
			socket(int fd, const std::string &host, const std::string &port);
			socket &operator=(const socket&);
			int get_fd() const;
			const std::string &get_host() const;
			const std::string &get_port() const;
			operator int() const;
	};
}

#endif
