#ifndef SERVER_HPP
# define SERVER_HPP

# include "stdafx.hpp"
# include "base_dir_ext.hpp"

namespace ft
{
	class server : public base_dir_ext
	{
		private:
			string_vector _names;
			int_vector _sockets;
		public:
			server();
			~server();
			server(const server &other);
			server(const base_dir &other);
			server &operator=(const server &other);
			const string_vector &get_names() const;
			const int_vector &get_sockets() const;
			void add_name(const std::string &name);
			void close_sockets();
			void add_socket(const std::string &host, const std::string &port);
	};
}

#endif
