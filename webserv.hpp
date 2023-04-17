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
			http *_protocol;
		public:
			webserv();
			~webserv();
			webserv(const webserv &other);
			webserv &operator=(const webserv &other);
			http &get_http(); // should it be const-qualified?
			void set_http(base_dir *protocol);
			int error(const std::string &error) const;
			int log(const std::string &message, const char *color = RESET) const;
			void start_service();
		private:
			int receive_request(request &request);
			int generate_response(request &request, int error = 0);
			int send_response(response &response);
	};
}

extern ft::webserv webserver;

#endif
