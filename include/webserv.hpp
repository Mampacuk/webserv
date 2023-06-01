#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "http.hpp"
# include "response.hpp"

namespace ft
{
	class webserv
	{
		private:
			http *_protocol;
		public:
			webserv();
			~webserv();
			webserv(const webserv &other);
			webserv &operator=(const webserv &other);
			const http &get_http() const;
			void set_http(base_dir *protocol);
			void serve() const;
			static int error(const std::string &error);
			static int log(const std::string &message, const char *color = RESET);
		private:
			client_socket accept_connection(const server_socket &socket) const;
			int receive_request(request &request, response_list &responses) const;
			int send_response(response &response) const;
	};
}

#endif
