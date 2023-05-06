#ifndef HTTP_HPP
# define HTTP_HPP

# include "stdafx.hpp"
# include "base_dir.hpp"
# include "socket.hpp"

namespace ft
{
	class http : public base_dir
	{
		private:
			server_socket_set	_sockets;
			server_list			_servers;
		public:
			http();
			~http();
			http(const http &other);
			http &operator=(const http &other);
			const server_list &get_servers() const;
			void add_server(server server);
			const server_socket_set &get_sockets() const;
			void add_socket(const socket &socket);
			void close_sockets();
			static bool is_port_number(const std::string &port_string);
			static bool is_informational_code(http_code status);
			static bool is_successful_code(http_code status);
			static bool is_redirection_code(http_code status);
			static bool is_client_error_code(http_code status);
			static bool is_server_error_code(http_code status);
			static bool is_error_code(http_code status);
			static std::string reason_phrase(http_code status);

			class protocol_error : public std::logic_error
			{
				private:
					http_code _error;
				public:
					protocol_error(http_code error, const std::string &what) : std::logic_error(what), _error(error) {}
					protocol_error(http_code error, const char *what) : std::logic_error(what), _error(error) {}
					operator http_code() const { return (this->_error); }
			};
	};
}

# include "server.hpp"

#endif
