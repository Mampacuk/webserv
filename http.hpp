#ifndef HTTP_HPP
# define HTTP_HPP

# include "stdafx.hpp"
# include "base_dir.hpp"

namespace ft
{
	class http : public base_dir
	{
		private:
			server_vector	servers;
		public:
			http();
			~http();
			http(const http &other);
			http &operator=(const http &other);
			const server_vector &get_servers() const;
			void add_server(server server);
			void close_server_sockets();
			int_string_map initialize_master(fd_set &master_set) const;
	};
}

# include "server.hpp"

#endif
