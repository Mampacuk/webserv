#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "stdafx.hpp"

namespace ft
{
	class request 
	{
		private:
			std::string         _method;
			std::string         _uri;
			string_map          _headers;
			std::string         _body;
            const std::string   &_raw;
			request();
			request(const request &other);
		public:
			~request();
			request &operator=(const request &other);
			request(const std::string &message);
			const std::string &get_body() const;
			void read_header_value(const std::string &header, int pos = std::string::npos);
            operator bool();
			// const std::string &get_header_value(const std::string &header) const;
	};
}

#endif