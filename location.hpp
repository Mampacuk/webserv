#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "stdafx.hpp"
# include "base_dir_ext.hpp"

namespace ft
{
	class location : public base_dir_ext
	{
		private:
			string_pair	_cgi;
			string_set	_methods; // if empty, everything is allowed
			std::string	_route;
			bool		_modifier;
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
			void set_route(const std::string &route, location *parent = NULL);
			void add_method(const std::string &method);
			void set_modifier(bool modifier);
			bool operator<(const location &rhs) const;
			void flush_cgi();
			void flush_methods();
	};
}

#endif
