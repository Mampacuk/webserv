#ifndef BASE_DIR_EXT_HPP
# define BASE_DIR_EXT_HPP

// # include "stdafx.hpp"
# include "base_dir.hpp"

namespace ft
{
	class base_dir_ext : public base_dir
	{
		protected:
			location_set	_locations;
			string_mmap		_rewrites;
		public:
			base_dir_ext();
			virtual ~base_dir_ext();
			base_dir_ext(const base_dir_ext &other);
			base_dir_ext(const base_dir &other);
			base_dir_ext &operator=(const base_dir_ext &other);
			const string_mmap &get_rewrites() const;
			const location_set &get_locations() const;
			void add_rewrite(std::string expression, std::string uri);
			void add_location(location location);
			void flush_rewrites();
	};
}

# include "location.hpp"

#endif
