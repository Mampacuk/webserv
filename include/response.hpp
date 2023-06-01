#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request.hpp"
# include "webserv.hpp"

namespace ft
{
	class response
	{
		private:
			request			_request;
			http_code		_status;
			string_map		_headers;
			std::string		_uri;
			std::string		_path;
			char_vector		_body;
			char_vector		_message;
			size_t			_cursor;
			std::string		_cgi;
			const location	*_location;
		public:
			response(const response &other);
			response(const request &request, http_code status = ok);
			~response();
		private:
			response &operator=(const response &other);
		public:
			operator int() const;
			char_vector_iterator_pair get_chunk();
			bool empty() const;
			bool sent() const;
		private:
			void generate_response();
			void construct_response();
			bool get_method();
			void read_error_page(http_code error);
			void construct_error_page(http_code error);
			void find_location(const base_dir_ext &level);
			void find_rewritten_location();
			void find_requested_file();
			bool read_requested_file(const std::string &file);
			void generate_autoindex(const std::string &path);
			void parse_pathinfo();
			void post_method(const std::string &cgi_executable);
			void cgi_process(const std::string &cgi_executable, int in[2], int out[2]);
			void delete_method();
			bool rewrite(const std::string &what, const std::string &with_what);
			std::string append_trailing_slash(const std::string &path) const;
			void print_response() const;
			bool exists(const std::string &filename) const;
			bool is_regular_file(const char *filename) const;
			bool is_directory(const char *filename) const;
	};
}

#endif
