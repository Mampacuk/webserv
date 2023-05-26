#include "request.hpp"

namespace ft
{
	request::~request() {}

	request &request::operator=(const request &other)
	{
		_method = other._method;
		_uri = other._uri;
		_query = other._query;
		_headers = other._headers;
		_raw = other._raw;
		_body = other._body;
		_socket = other._socket;
		_content_length = other._content_length;
		_headers_end = other._headers_end;
		_server = other._server;
		return (*this);
	}

	request::request(client_socket socket) : _method(), _uri(), _query(), _headers(), _raw(), _body(), _socket(socket), _content_length(-1), _headers_end(std::string::npos), _server() {}

	request::request(const request &other) : _method(other._method), _uri(other._uri), _query(other._query), _headers(other._headers), _raw(other._raw), _body(other._body),
		_socket(other._socket), _content_length(other._content_length), _headers_end(other._headers_end), _server(other._server) {}

	// appends chunk to the request. returns whether the request was fully accepted
	bool request::operator+=(const std::string &chunk)
	{
		_raw += chunk;
		if (_headers_end == std::string::npos)
		{
			if ((_headers_end = _raw.find(CRLF CRLF)) == std::string::npos)
			{
				std::cout << CYAN "NOT DONE RECEIVING HEADERS" RESET << std::endl; 
				return (false);
			}
			read_header(_raw.find("Content-Length: "));
			read_header(_raw.find("Transfer-Encoding: "));
			if (operator[]("Content-Length").empty())
			{
				if (operator[]("Transfer-Encoding").empty())
				{
					std::cout << CYAN "CONTENT-LENGTH ABSENT AND TRANSFER-ENCODING ABSENT" RESET << std::endl;
					return (true);
				}
				else if (operator[]("Transfer-Encoding") != "chunked") // the message is ill-formed
					throw protocol_error(bad_request, "Unsupported Transfer Encoding value.");
			}
			else
			{
				if (!operator[]("Transfer-Encoding").empty())
					throw protocol_error(bad_request, "Content Length and Transfer Encoding conflict.");
				_content_length = try_strtoul(operator[]("Content-Length"));
			}
		}
		if (_content_length < 0) // "Transfer-Encoding: chunked" case
		{
			std::cout << CYAN BOLDED("TRANSFER ENCODING CASE") RESET << std::endl;
			return (ends_with(_raw, "0" CRLF CRLF));
		}
		// std::cout << CYAN "COMPELTED RECEIVING REQUEST? " << ((_raw.size() == _content_length + _headers_end + std::strlen(CRLF CRLF CRLF)
		// 	|| (ends_with(_raw, CRLF) && !ends_with(_raw, CRLF CRLF))) ? "yes" : "no") << RESET << std::endl;
		// std::cout << RED "vvvvvvv received chunk of size " << chunk.size() << "vvvvvvv" RESET << std::endl;
		// std::cout << chunk << std::endl;
		// std::cout << RED "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" RESET << std::endl;
		// std::cout << LGREEN "raw size: " << _raw.size() << std::endl;
		// std::cout << "_content_length: " << _content_length << std::endl;
		// std::cout << "_headers_end: " << _headers_end << std::endl;
		// std::cout << "_content_length + _headers_end + std::strlen(CRLF CRLF CRLF): " << (_content_length + _headers_end + std::strlen(CRLF CRLF CRLF)) << std::endl;
		// std::cout << "(ends_with(_raw, CRLF): " << (ends_with(_raw, CRLF)) << std::endl;
		// std::cout << "!ends_with(_raw, CRLF CRLF)): " << !ends_with(_raw, CRLF CRLF) << RESET << std::endl;
		return (_raw.size() >= _content_length + _headers_end + std::strlen(CRLF CRLF)
			|| (ends_with(_raw, CRLF) && !ends_with(_raw, CRLF CRLF)));
	}

	// should only be called once _headers_end is initialized, i.e. all headers were received
	size_t request::read_header(size_t pos)
	{
		// verify the headers is present, it's not in the body, and it's preceded by a CRLF
		if (pos != std::string::npos && pos < _headers_end && !_raw.compare(pos - std::strlen(CRLF), std::strlen(CRLF), CRLF))
		{
			size_t line_end = _raw.find(CRLF, pos);
			size_t colon = _raw.find(':', pos);
			// check if there's a key, colon is present, there's no spaces in header and it's followed by one space
			if (colon == pos || colon > line_end || colon != _raw.find(' ', pos) - 1)
				throw protocol_error(bad_request, "Invalid Header.");
			std::string key = _raw.substr(pos, colon - pos);
			size_t val_start = _raw.find_first_not_of(' ', colon + 2);		// val start; 2 is to skip ": "
			std::string value = _raw.substr(val_start, line_end - val_start);	// val separated (with tail spaces)
			value = value.substr(0, value.find_last_not_of(' ') + 1);				// discard tail spaces
			_headers[key] = value;
			pos = line_end + std::strlen(CRLF);
		}
		return (pos);
	}

	void request::parse()
	{
		std::cout << "received request is" << std::endl << LRED << _raw << RESET << std::endl;
		separate_body();
		size_t pos = parse_request_line();
		// std::cout << "method: |" << _method << "|" << std::endl;
		// std::cout << "content length: " << _content_length << std::endl;
		// std::cout << "headers end: " << _headers_end << std::endl;
		// std::cout << "body: |" << _body << "|, empty? " << (_body.empty() ? "yes" : "no") << std::endl;
		while (pos != _headers_end + std::strlen(CRLF))
			pos = read_header(pos);
		// ... check for validity
		parse_query();
		// std::cout << CYAN "SELECTED SERVER?" RESET << std::endl;
		select_server();
		// std::cout << CYAN BOLDED("YES") RESET << std::endl;
		
		// std::cout << BLUE;
		// for (string_map::iterator it = _headers.begin(); it != _headers.end(); it++)
		// 	std::cout << "header: { " << it->first << " : " << it->second << " }" << std::endl;
		// std::cout << "uri: |" << _uri << "|" << std::endl;
		// std::cout << "query: |" << _query << "|" RESET << std::endl;
		_raw.clear();
	}

	void request::separate_body()
	{
		if (operator[]("Transfer-Encoding").empty())
		{
			const size_t read_body_start = _headers_end + std::strlen(CRLF CRLF);
			const size_t read_body_end = _raw.size() - (ends_with(_raw, CRLF) && !ends_with(_raw, CRLF CRLF) ? std::strlen(CRLF) : 0);
			_body += _raw.substr(read_body_start, read_body_end - read_body_start);
			if (_content_length == -1)
				_content_length = _body.size();
			else if (static_cast<size_t>(_content_length) != read_body_end - read_body_start)
				throw protocol_error(bad_request, "Content-Length mismatch.");
		}
		else
		{
			size_t	chunk_size;
			size_t	pos = _headers_end + std::strlen(CRLF CRLF);
			size_t	end_of_line = _raw.find(CRLF, pos);
			chunk_size = try_strtoul(_raw.substr(pos, end_of_line - pos), 16);
			_content_length = 0;
			while (chunk_size > 0)
			{
				pos = end_of_line + std::strlen(CRLF);	// now points to the beginning of chunk
				end_of_line = _raw.find(CRLF, pos);		// now points to the end of chunk
				if (end_of_line - pos != chunk_size)
					throw protocol_error(bad_request, "Chunk Size Mismatch.");
				_body += _raw.substr(pos, chunk_size);	// append the chunk to the body
				_content_length += chunk_size;
				pos = end_of_line + std::strlen(CRLF);	// now points to the beginning of chunk-size
				end_of_line = _raw.find(CRLF, pos);		// now points to the end of chunk-size
				chunk_size = try_strtoul(_raw.substr(pos, end_of_line - pos), 16);
			}
		}
		_raw.erase(_headers_end + std::strlen(CRLF CRLF));
	}

	size_t request::parse_request_line()
	{
		size_t line_end = _raw.find(CRLF);
		size_t space = _raw.find(' ');
		if (!space || space > line_end)
			throw protocol_error(bad_request, "Method unspecified.");
		_method = _raw.substr(0, space);
		space = _raw.find(' ', space + 1);
		if (space > line_end)
			throw protocol_error(bad_request, "URI unspecified.");
		_uri = _raw.substr(_method.length() + 1, space - _method.length() - 1);
		if (_raw.compare(space + 1, line_end - space - 1, HTTP_VERSION))
			throw protocol_error(http_version_not_supported, "Invalid Protocol Version.");
		return (line_end + std::strlen(CRLF));
	}

	void request::parse_query()
	{
		size_t question_mark = _uri.find('?');
		if (question_mark != std::string::npos)
		{
			_query = _uri.substr(question_mark + 1);
			_uri = _uri.substr(0, question_mark);
		}
	}

	void request::select_server()
	{
		const std::string hostname = operator[]("Host");
		if (hostname.empty())
			throw protocol_error(bad_request, "Host unspecified.");
		for (server_pointer_vector::const_iterator server = _socket.get_server_socket().get_servers().begin(); server != _socket.get_server_socket().get_servers().end(); server++)
			for (string_vector::const_iterator name = (*server)->get_names().begin(); name != (*server)->get_names().end(); name++)
			{
				if (*name == hostname)
				{
					_server = *server;
					return ;
				}
			}
		_server = _socket.get_server_socket().get_servers().front();
	}

	std::string request::operator[](const std::string &header) const
	{
		string_map::const_iterator header_pair = _headers.find(header);
		if (header_pair != _headers.end())
			return (header_pair->second);
		return ("");
	}

	const client_socket &request::get_socket() const
	{
		return (_socket);
	}

	const std::string &request::get_method() const
	{
		return (_method);
	}

	const std::string &request::get_uri() const
	{
		return (_uri);
	}

	const std::string &request::get_query() const
	{
		return (_query);
	}

	const std::string &request::get_body() const
	{
		return (_body);
	}

	ssize_t request::get_content_length() const
	{
		return (_content_length);
	}

	const server &request::get_server() const
	{
		if (!_server)
			throw std::runtime_error("Querying an incomplete request for its server is not allowed.");
		return (*_server);
	}

	request::operator int() const
	{
		return (_socket);
	}

	unsigned int request::try_strtoul(const std::string &number, int base) const
	{
		try
		{
			return (strtoul(number, base));
		}
		catch (const std::exception &e)
		{
			throw protocol_error(bad_request, "Unsigned integer parsing failed.");
		}
	}
}
