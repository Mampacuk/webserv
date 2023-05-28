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
			server(const base_dir_ext &other);
			server &operator=(const server &other);
			const string_vector &get_names() const;
			void add_name(const std::string &name);
	};
}

#endif
