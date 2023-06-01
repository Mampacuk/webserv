#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "client_socket.hpp"

namespace ft
{
	class request 
	{
		private:
			std::string		_method;
			std::string		_uri;
			std::string 	_query;
			std::string		_pathinfo;
			string_map		_headers;
			char_vector		_raw;
			char_vector 	_body;
			client_socket	_socket;
			ssize_t			_content_length;
			size_t			_headers_end;
			const server	*_server;
		public:
			request(client_socket socket);
			~request();
			request(const request &other);
			request &operator=(const request &other);
			bool append_chunk(const char *chunk, size_t chunk_size);
			void parse();
			std::string operator[](const std::string &header) const;
			const std::string &get_method() const;
			const std::string &get_uri() const;
			const std::string &get_query() const;
			const std::string &get_pathinfo() const;
			const string_map &get_headers() const;
			const char_vector &get_body() const;
			const client_socket &get_socket() const;
			ssize_t get_content_length() const;
			const server &get_server() const;
			void set_pathinfo(const std::string &pathinfo);
			void erase_header(const std::string &header);
			operator int() const;
		private:
			unsigned int	try_strtoul(const std::string &number, int base = 10) const;
			size_t			read_header(size_t pos);
			void			separate_body();
			size_t			parse_request_line();
			void			parse_query();
			void			select_server();
			void			print_request() const;
			void			validate_hostname(const std::string &hostname) const;
			size_t			parse_chunk_size(const std::string &field) const;
			void			validate_token(const std::string &token) const;
			void			validate_ext_val(const std::string &ext_val) const;
	};
}

#endif
