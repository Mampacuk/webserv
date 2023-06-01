#ifndef HTTP_HPP
# define HTTP_HPP

# include "client_socket.hpp"

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
			const server_socket_set &get_sockets() const;
			const server_list		&get_servers() const;
			void add_server(const server &server);
			void add_socket(const server_socket &socket);
			static bool is_port_number(const std::string &port_string);
			static bool is_informational_code(http_code status);
			static bool is_successful_code(http_code status);
			static bool is_redirection_code(http_code status);
			static bool is_client_error_code(http_code status);
			static bool is_server_error_code(http_code status);
			static bool is_error_code(http_code status);
	};
}

#endif
