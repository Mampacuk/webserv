#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request.hpp"
# include "stdafx.hpp"

namespace ft
{
	class response
	{
		private:
			http::code		_status;
			std::string		_body;
			string_map		_headers;
			std::string		_message;
			request			_request;
			size_t			_cursor; // indicates bytes already sent
			response &operator=(const response &other);
			response();
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
			void construct_response();
			void generate_response();
			void requested_file();
			void process_get();
			void process_post();
			void read_requested_file();
			void find_error_page();
			std::string status_to_string(int status_code) const;
	};
}

#endif
