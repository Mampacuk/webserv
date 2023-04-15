#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "stdafx.hpp"

namespace ft
{
	class request 
	{
		public:
			class protocol_error : std::logic_error { };
		private:
			std::string	_method;
			std::string	_uri;
			string_map	_headers;
			std::string	_body;
			std::string	_raw;
			int			_socket;
			int			_content_length;
			size_t		_headers_end;
			request();
			request(const request &other);
		public:
			~request();
			request &operator=(const request &other);
			request(int socket);
			const std::string &get_body() const;
			void read_header_value(const std::string &header, int pos = std::string::npos);
			bool operator+=(const std::string &chunk);
			std::string operator[](const std::string &header) const;
			int operator*() const;
	};
}

#endif
