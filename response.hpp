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
			// response();
			response &operator=(const response &other);
		public:
			response(const response &other);
			response(const request &request, int status_code = HTTP_STATUS_OK);
			~response();
			operator int() const;
			std::string to_string() const;
		private:
			std::string status_to_string(int status_code) const;
	};
}

#endif
