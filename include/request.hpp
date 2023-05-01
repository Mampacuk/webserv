#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "stdafx.hpp"
# include "http.hpp"
# include "parser.hpp"
# include "socket.hpp"

namespace ft
{
	class request 
	{
		private:
			std::string		_method;
			std::string		_uri;
			std::string 	_query;
			string_map		_headers;
			std::string		_raw;
			std::string 	_body;
			socket			_socket;
			int				_content_length;
			size_t			_headers_end;
			const server	*_server;
		public:
			request(socket socket);
			~request();
			request(const request &other);
			request &operator=(const request &other);
			bool operator+=(const std::string &chunk);
			std::string operator[](const std::string &header) const;
			void parse();
			socket get_socket() const;
			const std::string &get_method() const;
			const std::string &get_uri() const;
			const server &get_server() const;
			operator int() const;
		private:
			unsigned int try_strtoul(const std::string &number, int base = 10) const;
			void	find_header(const std::string &header);
			size_t	read_header(size_t pos);
			void	separate_body();
			size_t	parse_request_line();
			void	parse_query();
			void	select_server();
	};
}

#endif
