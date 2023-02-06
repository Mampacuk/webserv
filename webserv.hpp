#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "http.hpp"
# include <iostream>

namespace ft
{
	class webserv
	{
		private:
			http *protocol;
		public:
			webserv();
			~webserv();
			webserv(const webserv &other);
			webserv &operator=(const webserv &other);
			http &get_http();
			void set_http(base_dir *protocol);
			void verify_http();
			void print_base_dir(base_dir *ptr);
			int error(const std::string &message) const;
	};
}

extern ft::webserv webserv;

#endif
