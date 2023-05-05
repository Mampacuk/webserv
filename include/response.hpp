#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "stdafx.hpp"
# include "request.hpp"
# include "webserv.hpp"

namespace ft
{
	class response
	{
		private:
			http::code		_status;
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
			response(const request &request, http::code status = http::code::ok);
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
			void find_error_page();
			std::string status_to_string(int status_code) const;
			void find_rewritten_location();
			bool rewrite(const std::string &portion, const std::string &withwhat);
			void find_path();
			void generate_autoindex(const std::string &path);
			void execute_cgi(char *cgi_path, char **cgi_args, char **cgi_env);
			void malloc_safe_syscall(void *memory, void *mem1 = NULL, void *mem2 = NULL, void *mem3 = NULL, void *mem4 = NULL);
			void free_cgi_args();
			void pipe_safe_syscall(int status, int in_pipe[2], int out_pipe[2]);
	};
}

#endif
