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
			size_t			_cursor; // indicates bytes already sent
			const location	*_location;
			response &operator=(const response &other);
		public:
			~response();
			response(const response &other);
			response(const request &request, http_code status = ok);
			int get_socket() const;
			operator int() const;
			char_vector_iterator_pair get_chunk();
			bool empty() const;
			bool sent() const;
		private:
			void find_location(const base_dir_ext &level);
			void construct_response();
			void generate_response();
			void get_method();
			void post_method(const std::string &cgi_executable);
			void delete_method();
			void find_requested_file();
			bool read_requested_file(const std::string &file);
			void find_rewritten_location();
			bool rewrite(const std::string &what, const std::string &with_what);
			void generate_autoindex(const std::string &path);
			void cgi_process(const std::string &cgi_executable, int fds[2]);
			void read_error_page(http_code error, bool loc = true);
			void construct_error_page(http_code error);
			bool is_regular_file(const char *filename) const;
			bool is_directory(const char *filename) const;
			std::string append_trailing_slash(const std::string &path);
	};
}

#endif
