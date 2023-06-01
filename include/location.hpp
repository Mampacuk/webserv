#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "base_dir_ext.hpp"

namespace ft
{
	class location : public base_dir_ext
	{
		private:
			std::string	_route;
			bool		_modifier;
			string_set	_methods; // empty => everything allowed
			bool		_flush_methods;
		public:
			location();
			~location();
			location(const location &other);
			location(const base_dir &other, const std::string &route = "");
			location &operator=(const location &other);
			const std::string &get_route() const;
			bool has_modifier() const;
			bool is_allowed_method(const std::string &method) const;
			void set_route(const std::string &route, location *parent = NULL);
			void set_modifier(bool modifier);
			void add_method(const std::string &method);
			void flush_methods();
			bool operator<(const location &rhs) const;
			bool operator==(const location &rhs) const;
	};
}

#endif
