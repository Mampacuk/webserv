#ifndef BASE_DIR_HPP
# define BASE_DIR_HPP

# include "stdafx.hpp"

namespace ft
{
	class base_dir
	{
		protected:
			bool				_autoindex;
			std::string			_root;
			// std::string			_cgi_executable;
			// std::string			_cgi_extension;
			unsigned long int	_client_max_body_size; // in bytes
		public:	
			string_map			_cgi;
		protected:	
			error_map			_error_pages;
			string_vector		_indices;
			bool				_flush_cgi;
			bool				_flush_error_pages;
			bool				_flush_indices;
		public:
			base_dir();
			virtual ~base_dir();
			base_dir(const base_dir &other);
			base_dir &operator=(const base_dir &other);
			void set_root(const std::string &root);
			void set_autoindex(bool autoindex);
			void set_client_max_body_size(unsigned long int size);
			void add_cgi(const std::string &cgi_extension, const std::string &cgi_executable);
			// void set_cgi_executable(const std::string &cgi_executable);
			// void set_cgi_extension(const std::string &cgi_extension);
			const std::string &get_root() const;
			bool get_autoindex() const;
			unsigned int get_client_max_body_size() const;
			std::string get_cgi_executable(const std::string &cgi_extension) const;
			const string_map &get_cgi() const;
			// const std::string &get_cgi_extension() const;
			void add_error_page(http_code error, const std::string &page);
			void add_index(const std::string &index_file);
			std::string get_error_page(http_code error) const;
			const string_vector &get_indices() const;
			void flush_cgi();
			void flush_error_pages();
			void flush_indices();
	};
}

#endif