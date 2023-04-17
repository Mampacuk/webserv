#ifndef HTTP_HPP
# define HTTP_HPP

# include "stdafx.hpp"
# include "base_dir.hpp"

namespace ft
{
	class http : public base_dir
	{
		private:
			server_vector	_servers;
		public:
			http();
			~http();
			http(const http &other);
			http &operator=(const http &other);
			const server_vector &get_servers() const;
			void add_server(server server);
			void close_server_sockets();
			int_set initialize_master(fd_set &master_set) const;
			class protocol_error : std::logic_error
			{
				private:
					int _error_code;
				public:
					explicit protocol_error(int error_code, const std::string &what) : std::logic_error(what), _error_code(error_code) {}
					explicit protocol_error(int error_code, const char *what) : std::logic_error(what), _error_code(error_code) {}
					operator int() const { return (this->_error_code); }
			};
	};
}

# include "server.hpp"

#endif
