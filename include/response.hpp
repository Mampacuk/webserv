#ifndef RESPONSE_HPP
# define RESPONSE_HPP

// # include "stdafx.hpp" // included in request
# include "request.hpp"
# include "webserv.hpp" //?

namespace ft
{
	class response
	{
		private:
			http_code		_status;
			std::string		_body;
			string_map		_headers;
			std::string		_message;
			std::string		_uri;
			request			_request;
			const location	*_location;
			size_t			_cursor; // indicates bytes already sent
			std::string		_path;
			response &operator=(const response &other);
		public:
			~response();
			response(const response &other);
			response(const request &request, http_code status = ok);
			int get_socket() const;
			operator int() const;
			std::string get_chunk();
			bool empty() const;
			bool sent() const;
		private:
			void find_location();
			void construct_response();
			void generate_response();
			void get();
			void post();
			void find_requested_file();
			bool read_requested_file(const std::string &file);
			void find_rewritten_location();
			bool rewrite(const std::string &what, const std::string &with_what);
			// void find_path();
			void generate_autoindex(const std::string &path);
			void execute_cgi(char *cgi_path, char **cgi_args, char **cgi_env);
			void pipe_safe_syscall(int status, int in_pipe[2] = NULL, int out_pipe[2] = NULL);
			void malloc_safe_syscall(void *memory, void *mem1 = NULL, void *mem2 = NULL, void *mem3 = NULL, void *mem4 = NULL);
			void free_cgi_args(char *cgi_path, char **cgi_args, char **cgi_env);
			void read_error_page(int error_code, bool loc = true);
			void construct_error_page(int error_code);
	};
}

#endif
