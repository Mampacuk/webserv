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
		std::set<std::string> methods;				//if empty everything is allowed
		std::string rout;
		bool modifier;
	public:
		location();
		~location();
		location(const location &other);
		location(const base_dir &other);
		location &operator=(const location &other);
		const std::string get_cgi(const std::string extension) const;
		void add_cgi(const std::string &extension, const std::string &path);
		const std::string &get_rout() const;
		bool method_allowed(const std::string &method) const;
		void add_method(const std::string &method);
		bool has_modifier() const;
};

#endif
