#ifndef BASE_DIR_HPP
# define BASE_DIR_HPP

# include <string>
# include <map> 
# include <vector>

namespace ft
{
	typedef std::vector<std::string> string_vector;
	typedef std::map<unsigned int, std::string> error_map;
	typedef std::pair<unsigned int, std::string> error_page;

	class base_dir
	{
		protected:
			std::string root;
			bool autoindex;
			error_map error_pages;
			unsigned long int client_max_body_size; // in bytes
			string_vector indexes;
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
			const std::string get_error_page(unsigned int error_code) const;
			const string_vector &get_indexes() const;
			void flush_error_pages();
			void flush_indexes();

			 // remove later
			error_map &get_error_map()
			{
				return (this->error_pages);
			}
	};
}

#endif