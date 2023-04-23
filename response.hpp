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
			std::string _body;
			string_map	_headers;
			const 		request &_req;
			//...
			// response();
		public:
			response(const response &other);
			response(const request &request);
			~response();
			response &operator=(const response &other);
			int get_socket() const;
			operator int() const;
			std::string to_string() const;
			//...
	};
}

#endif
