#ifndef SERVER_HPP
# define SERVER_HPP

# include "base_dir_ext.hpp"
# include "location.hpp"

namespace ft
{
	typedef std::set<std::string> string_set;
	typedef std::pair<std::string, unsigned int> address;
	typedef std::set<address> address_set;

	class server : public base_dir_ext
	{
		private:
			string_set names;
			address_set listens;
		public:
			server();
			~server();
			server(const server &other);
			server(const base_dir &other);
			server &operator=(const server &other);
			const string_set &get_names() const;
			const address_set &get_listens() const;
			void add_name(const std::string &name);
			void add_listen(const std::string &host, unsigned int port);
	};
}

#endif
