#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <set>
# include <map>
# include <stdexcept>

# include "base_dir_ext.hpp"

namespace ft
{
	class server;

	typedef std::map<std::string, std::string> string_map;
	typedef std::set<std::string> string_set;
	typedef std::pair<std::string, std::string> string_pair;

	class location : public base_dir_ext
	{
		private:
			string_pair cgi;
			string_set methods;		//if empty, everything is allowed
			std::string route;
			bool modifier;
		public:
			location();
			~location();
			location(const location &other);
			location(const base_dir &other);
			location &operator=(const location &other);
			const string_pair &get_cgi() const;
			const std::string &get_route() const;
			bool method_allowed(const std::string &method) const;
			bool has_modifier() const;
			void add_cgi(const std::string &extension, const std::string &path);
			void set_route(const std::string &route);
			void add_method(const std::string &method);
			void set_modifier(bool modifier);
			bool operator<(const location &rhs) const;
			void flush_cgi();
			void flush_methods();

			//delete later
			const string_set &get_methods() const
			{
				return (this->methods);
			}
	};
}

#include "server.hpp"
#endif
