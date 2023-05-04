#ifndef BASE_DIR_HPP
# define BASE_DIR_HPP

# include "stdafx.hpp"

namespace ft
{
	class base_dir
	{
		protected:
			bool				_autoindex;
			error_map			_error_pages;
			std::string			_root;
			string_vector		_indexes;
			unsigned long int	_client_max_body_size; // in bytes
			string_map			_cgi_params;
		public:
			base_dir();
			virtual ~base_dir();
			base_dir(const base_dir &other);
			base_dir &operator=(const base_dir &other);
			void set_root(const std::string &root);
			void set_autoindex(bool autoindex);
			void set_client_max_body_size(unsigned long int size);
			const std::string &get_root() const;
			bool get_autoindex() const;
			unsigned int get_client_max_body_size() const;
			void add_error_page(unsigned int error_code, const std::string &page);
			void add_index(const std::string &index_file);
			void add_cgi_param(const std::string &key, const std::string &value);
			std::string get_error_page(unsigned int error_code) const;
			std::string get_cgi_param(const std::string &key) const;
			const string_vector &get_indexes() const;
			const string_map &get_cgi_params() const;
			void flush_error_pages();
			void flush_indexes();
			void flush_cgi_params();
	};
}

#endif