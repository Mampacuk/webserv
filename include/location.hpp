#ifndef LOCATION_HPP
# define LOCATION_HPP

// # include "stdafx.hpp"
# include "base_dir_ext.hpp"

namespace ft
{
	class location : public base_dir_ext
	{
		private:
			string_set	_methods; // if empty, everything is allowed
			std::string	_route;
			bool		_modifier;
		public:
			location();
			~location();
			location(const location &other);
			location(const base_dir &other, const std::string &route = "");
			location &operator=(const location &other);
			bool operator<(const location &rhs) const;
			bool operator==(const location &rhs) const;
			const std::string &get_route() const;
			bool is_allowed_method(const std::string &method) const;
			bool has_modifier() const;
			void set_route(const std::string &route, location *parent = NULL);
			void add_method(const std::string &method);
			void set_modifier(bool modifier);
			void flush_cgi_params();
			void flush_methods();
	};
}

#endif
