#ifndef BASE_DIR_EXT_HPP
# define BASE_DIR_EXT_HPP

# include "base_dir.hpp"

namespace ft
{
	class base_dir_ext : public base_dir
	{
		protected:
			location_set	_locations;
			string_map		_rewrites;
		public:
			base_dir_ext();
			virtual ~base_dir_ext();
			base_dir_ext(const base_dir_ext &other);
			base_dir_ext(const base_dir &other);
			base_dir_ext &operator=(const base_dir_ext &other);
			const location_set &get_locations() const;
			const string_map &get_rewrites() const;
			void add_location(location location);
			void add_rewrite(std::string expression, std::string uri);
			void flush_rewrites();
	};
}

# include "location.hpp"

#endif
