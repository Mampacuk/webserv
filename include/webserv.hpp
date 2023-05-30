#ifndef WEBSERV_HPP
# define WEBSERV_HPP

// # include "stdafx.hpp"
# include "http.hpp"
// # include "request.hpp" //included from response.cpp
# include "response.hpp"
// # include "parser.hpp"

namespace ft
{
	class webserv
	{
		private:
			http	*_protocol;
		public:
			webserv();
			~webserv();
			webserv(const webserv &other);
			webserv &operator=(const webserv &other);
			const http &get_http() const;
			void set_http(base_dir *protocol);
			void set_environment(char **environment);
			static int error(const std::string &error);
			static int log(const std::string &message, const char *color = RESET);
			static int label_log(const std::string &msg, const std::string &label, const char *msg_color = RESET, const char *label_color = YELLOW);
			void serve();
		private:
			client_socket accept_connection(const server_socket &socket);
			int receive_request(request &request, response_list &responses);
			int send_response(response &response);
	};
}

extern ft::webserv webserver;

#endif
