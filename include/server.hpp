#ifndef SERVER_HPP
# define SERVER_HPP

// # include "stdafx.hpp"
// # include "base_dir_ext.hpp"
# include "location.hpp"

namespace ft
{
	class server : public base_dir_ext
	{
		private:
			string_vector _names;
		public:
			server();
			~server();
			server(const server &other);
			server(const base_dir &other);
			server &operator=(const server &other);
			const string_vector &get_names() const;
			void add_name(const std::string &name);

			class server_error : public std::logic_error
			{
				private:
					http_code _error;
				public:
					explicit server_error(http_code error, const std::string &what) : std::logic_error(what), _error(error) {}
					explicit server_error(http_code error, const char *what) : std::logic_error(what), _error(error) {}
					operator http_code() const { return (this->_error); }
			};
	};
}

#endif
