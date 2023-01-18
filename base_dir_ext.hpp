#ifndef BASE_DIR_EXT_HPP
# define BASE_DIR_EXT_HPP

# include "base_dir.hpp"
# include "location.hpp"

class base_dir_ext : public base_dir
{
	protected:
		std::set<location> locations;
		std::multimap<std::string, std::string> redirects;
	public:
		base_dir_ext();
		virtual ~base_dir_ext();
		base_dir_ext(const base_dir_ext &other);
		base_dir_ext &operator=(const base_dir_ext &other);
		const std::multimap<std::string, std::string> &get_redirects() const;
		const std::set<location> &get_locations() const;
		void add_redirect(std::string expression, std::string uri);
		void add_location(location location);
};

#endif
