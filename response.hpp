#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "request.hpp"
# include "stdafx.hpp"

namespace ft
{
	class response
	{
		private:
			int 		_status;
			string_map	_headers;
			std::string _body;
			const 		request &_req;
			std::string _response;
			// response();
			response &operator=(const response &other);
			response(const response &other);
		public:
			response(const request &request, int status_code = HTTP_STATUS_OK);
			~response();
			operator int() const;
		private:
			void construct_response();
			void generate_response();
			void find_error_page();
			bool is_error_code(int status_code);
			std::string status_to_string(int status_code) const;
	};
}

#endif
