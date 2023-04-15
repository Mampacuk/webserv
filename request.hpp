#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "stdafx.hpp"
# include "parser.hpp"

namespace ft
{
	class request 
	{
		private:
			std::string	_method;
			std::string	_uri;
			string_map	_headers;
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
			void read_header_value(const std::string &header, int pos = std::string::npos);
			bool operator+=(const std::string &chunk);
			std::string operator[](const std::string &header) const;
			int get_socket() const;
			void decode_chunked_transfer();
		public:
			class protocol_error : std::logic_error
			{
				public:
					explicit protocol_error(const std::string &what) : std::logic_error(what) {}
					explicit protocol_error(const char *what) : std::logic_error(what) {}
			};
	};
}

#endif
