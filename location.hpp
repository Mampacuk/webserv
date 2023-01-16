#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "base_dir_ext.hpp"

# include <string>
# include <set>
# include <map>

class location : public base_dir_ext
{
	private:
		std::map<std::string, std::string> cgi;
		std::set<std::string> methods;
	public:
		location();
		~location();
		location(const location &other);
		location(const base_dir &other);
		location &operator=(const location &other);
		const std::string &get_cgi(const std::string extension);
		bool method_allowed(const std::string &method);
};

#endif
