#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <cstdlib>
# include <iostream>

# include "http.hpp"

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
			http &get_http(); // should it be const-qualified?
			void set_http(base_dir *protocol);
			void verify_http();
			int error(const std::string &message) const;
		private:
			void print_base_dir_ext(base_dir_ext *ptr);
			void print_base_dir(base_dir *ptr);
	};
}

extern ft::webserv webserv;

#endif
