#ifndef BASE_DIR_EXT_HPP
# define BASE_DIR_EXT_HPP

# include "base_dir.hpp"
# include <set>

namespace ft
{
	class location;

	typedef std::set<location> location_set;
	typedef std::multimap<std::string, std::string> string_mmap;

	class base_dir_ext : public base_dir
	{
		protected:
			location_set locations;
			string_mmap redirects;
		public:
			base_dir_ext();
			virtual ~base_dir_ext();
			base_dir_ext(const base_dir_ext &other);
			base_dir_ext(const base_dir &other);
			base_dir_ext &operator=(const base_dir_ext &other);
			const string_mmap &get_redirects() const;
			const location_set &get_locations() const;
			void add_redirect(std::string expression, std::string uri);
			void add_location(location location);
	};
}

#include "location.hpp"

#endif
