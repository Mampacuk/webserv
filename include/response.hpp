#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "stdafx.hpp"
# include "request.hpp"

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
			void find_rewritten_location();
			bool rewrite(const std::string &portion, const std::string &withwhat);
			// void find_path();
			void generate_autoindex(const std::string &path);
			void read_error_page(int error_code, bool loc = true);
			void construct_error_page(int error_code);
	};
}

#endif
