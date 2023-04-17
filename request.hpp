#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "stdafx.hpp"
# include "http.hpp"
# include "parser.hpp"

namespace ft
{
	class request 
	{
		// private:
		public:
			std::string	_method;
			std::string	_uri;
			std::string _query;
			string_map	_headers;
			std::string	_raw;
			std::string _body;
			int			_socket;
			int			_content_length;
			size_t		_headers_end;
			request();
			request(const request &other);
		public:
			request(int socket);
			~request();
			request &operator=(const request &other);
			bool operator+=(const std::string &chunk);
			std::string operator[](const std::string &header) const;
			int get_socket() const;
			operator int() const;
			void parse();
		private:
			unsigned int try_strtoul(const std::string &number, int base = 10) const;
			void	find_header(const std::string &header);
			size_t	read_header(size_t pos);
			void	separate_body();
			size_t	parse_request_line();
			void	parse_query();
	};
}

#endif
