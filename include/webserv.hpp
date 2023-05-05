#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "stdafx.hpp"
# include "http.hpp"
# include "request.hpp"
# include "response.hpp"
# include "parser.hpp"

namespace ft
{
	class webserv
	{
		private:
			http	*_protocol;
			char	**_environ;
		public:
			webserv();
			~webserv();
			webserv(const webserv &other);
			webserv &operator=(const webserv &other);
			const http &get_http() const;
			char **get_environ() const;
			void set_http(base_dir *protocol);
			void set_environ(char **environ);
			int error(const std::string &error) const;
			int log(const std::string &message, const char *color = RESET) const;
			void start_service();
		private:
			client_socket accept_connection(const server_socket &socket);
			int receive_request(request &request, response_list &responses);
			int send_response(response &response);
	};
}

extern ft::webserv webserver;

#endif
