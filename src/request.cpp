#include "request.hpp"

namespace ft
{
	request::~request() {}

	request &request::operator=(const request &other)
	{
		_method = other._method;
		_uri = other._uri;
		_query = other._query;
		_pathinfo = other._pathinfo;
		_headers = other._headers;
		_raw = other._raw;
		_body = other._body;
		_socket = other._socket;
		_content_length = other._content_length;
		_headers_end = other._headers_end;
		_chunk_size_begin = other._chunk_size_begin;
		_server = other._server;
		return (*this);
	}

	request::request(client_socket socket) : _method(), _uri(), _query(), _pathinfo(), _headers(), _raw(), _body(), _socket(socket), _content_length(-1), _headers_end(std::string::npos), _chunk_size_begin(0), _server()
	{
		_server = _socket.get_server_socket().get_servers().front();
	}

	request::request(const request &other) : _method(other._method), _uri(other._uri), _query(other._query), _pathinfo(other._pathinfo), _headers(other._headers), _raw(other._raw), _body(other._body),
		_socket(other._socket), _content_length(other._content_length), _headers_end(other._headers_end), _chunk_size_begin(other._chunk_size_begin), _server(other._server) {}

	// appends chunk to the request. returns whether the request was fully accepted
	bool request::append_chunk(const char *chunk, size_t chunk_size)
	{
		_raw.insert(_raw.end(), chunk, chunk + chunk_size);
		// std::cout << MAGENTA BOLDED("------- start of received CHUNK -------") << MAGENTA << std::endl;
		// write(STDOUT_FILENO, chunk, chunk_size);
		// std::cout << std::endl << MAGENTA BOLDED("-------- END of received CHUNK --------") RESET << std::endl;
		if (_headers_end == std::string::npos)
		{
			if ((_headers_end = search(_raw, std::string(CRLF CRLF))) == std::string::npos)
			{
				// std::cout << CYAN "NOT DONE RECEIVING HEADERS" RESET << std::endl; 
				return (false);
			}
			read_header(search(_raw, std::string("Content-Length:"), 0, case_insensitive_equal_to()));
			read_header(search(_raw, std::string("Transfer-Encoding:"), 0, case_insensitive_equal_to()));
			if (operator[]("Content-Length").empty())
			{
				if (operator[]("Transfer-Encoding").empty())
				{
					// check with small BUFSIZ
					// std::cout << CYAN "CONTENT-LENGTH ABSENT AND TRANSFER-ENCODING ABSENT" RESET << std::endl;
					return (true);
				}
				else if (operator[]("Transfer-Encoding") != "chunked") // the message is ill-formed
					throw protocol_error(bad_request);
				// std::cout << CYAN "TRANSFER-ENCODING CASE" RESET << std::endl;
			}
			else
			{
				if (!operator[]("Transfer-Encoding").empty())
					throw protocol_error(bad_request);
				_content_length = try_strtoul(operator[]("Content-Length"));
			}
			erase_header("Content-Length");
			erase_header("Transfer-Encoding");
		}
		if (_content_length < 0) // "Transfer-Encoding: chunked" case
		{
			std::cout << CYAN BOLDED("TRANSFER ENCODING CASE") RESET << std::endl;
			return (seek_chunk());
		}
		// std::cout << CYAN "COMPLETED RECEIVING REQUEST? " << ((_raw.size() == _content_length + _headers_end + std::strlen(CRLF CRLF CRLF)
		// 	|| (ends_with(_raw, CRLF) && !ends_with(_raw, CRLF CRLF))) ? "yes" : "no") << RESET << std::endl;
		// std::cout << RED "vvvvvvv received chunk of size " << chunk_size << "vvvvvvv" RESET << std::endl;
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
		if (pos != std::string::npos && pos < _headers_end && std::equal(_raw.begin() + pos - std::strlen(CRLF), _raw.begin() + pos, CRLF))
		{
			size_t line_end = search(_raw, std::string(CRLF), pos);
			if (line_end == std::string::npos)
				throw protocol_error(bad_request);
			std::string line = std::string(_raw.begin() + pos, _raw.begin() + line_end);
			if (line.empty())
				return (std::string::npos);
			size_t colon = line.find(':');
			if (colon == 0 || colon == std::string::npos)
				throw protocol_error(bad_request);
			// std::cout << CYAN "bad_request1" RESET << std::endl;
			std::string key = uppercase(std::string(_raw.begin() + pos, _raw.begin() + pos + colon));
			if (_headers.find(key) != _headers.end())
				throw protocol_error(bad_request);
			// std::cout << CYAN "bad_request2" RESET << std::endl;
			size_t val_start = line.find_first_not_of(' ', colon + 1);
			std::string value = (val_start == std::string::npos ? "" : line.substr(val_start));	// val separated (with tail spaces)
			value = value.substr(0, value.find_last_not_of(' ') + 1);							// discard tail spaces
			_headers[key] = value;
			pos = line_end + std::strlen(CRLF);
		}
		return (pos);
	}

	void request::parse()
	{
		print_request();
		separate_body();
		// std::cout << MAGENTA "separated body" RESET << std::endl;
		size_t pos = parse_request_line();
		// std::cout << MAGENTA "parsed request line" RESET << std::endl;
		_headers_end = std::string::npos; // to allow trailer search
		while (pos != std::string::npos)
			pos = read_header(pos);
		// std::cout << MAGENTA "parsed headers" RESET << std::endl;
		select_server();
		// std::cout << MAGENTA "selected server" RESET << std::endl;
		parse_query();
		// std::cout << MAGENTA "parsed query" RESET << std::endl;
		_raw.clear();
	}

	// returns true if the trailing 0-chunk was found
	// appends chunk to body, throws bad_request if there's chunk size mismatch
	// sets _chunk_size_begin to point to the start of the trailer once 0-chunk is found
	bool request::seek_chunk()
	{
		if (!_chunk_size_begin)
			_chunk_size_begin = _headers_end + std::strlen(CRLF CRLF);
		const size_t chunk_size_end = search(_raw, std::string(CRLF), _chunk_size_begin);
		if (chunk_size_end == std::string::npos)
			return (false);
		const size_t chunk_size = parse_chunk_size(std::string(_raw.begin() + _chunk_size_begin, _raw.begin() + chunk_size_end));
		if (chunk_size == 0)
		{
			_chunk_size_begin = chunk_size_end + std::strlen(CRLF);
			return (true);
		}
		const size_t chunk_content_begin = chunk_size_end + std::strlen(CRLF);
		const size_t chunk_content_end = search(_raw, std::string(CRLF), chunk_content_begin);
		if (chunk_content_end == std::string::npos)
			return (false);
		if (chunk_content_end - chunk_content_begin != chunk_size)
			throw protocol_error(bad_request);
		_body.insert(_body.end(), _raw.begin() + chunk_content_begin, _raw.begin() + chunk_content_end);
		_chunk_size_begin = chunk_content_end + std::strlen(CRLF);
		return (false);
	}

	void request::separate_body()
	{
		size_t trailer_begin = 0;

		read_header(search(_raw, std::string("Transfer-Encoding:"), 0, case_insensitive_equal_to()));
		if (operator[]("Transfer-Encoding").empty())
		{
			const size_t read_body_start = _headers_end + std::strlen(CRLF CRLF);
			const size_t read_body_end = _raw.size();
			_body.assign(_raw.begin() + read_body_start, _raw.begin() + read_body_end);
			if (_content_length == -1)
				_content_length = _body.size();
			else if (static_cast<size_t>(_content_length) != read_body_end - read_body_start)
				throw protocol_error(bad_request);
			_raw.erase(_raw.begin() + _headers_end + std::strlen(CRLF CRLF), _raw.end());
		}
		else
		{
			_content_length = _body.size();
			_raw.erase(_raw.begin() + _headers_end + std::strlen(CRLF), _raw.begin() + _chunk_size_begin);
		}
		// std::cout << YELLOW "body (size " << _body.size() << ") now" << std::endl;
		// write(STDOUT_FILENO, &_body.front(), _body.size());
		// std::cout << RESET << std::endl;
		// std::cout << YELLOW "raw (size " << _raw.size() << ") now |";
		// for (size_t i = 0; i < _raw.size(); i++) std::cout << _raw[i];
		// std::cout << "|" << RESET << std::endl;
		// std::cout << RED << "ends with double CRLF?" << ends_with(_raw, CRLF CRLF) << std::endl;
		erase_header("Transfer-Encoding");
	}

	size_t request::parse_request_line()
	{
		size_t line_end = search(_raw, std::string(CRLF));
		size_t space = search(_raw, std::string(" "));
		if (!space || space > line_end)
			throw protocol_error(bad_request);
		_method = std::string(_raw.begin(), _raw.begin() + space);
		space = search(_raw, std::string(" "), space + 1);
		if (space > line_end)
			throw protocol_error(bad_request);
		_uri = std::string(_raw.begin() + _method.length() + 1, _raw.begin() + space);
		if (_uri.empty())
			throw protocol_error(bad_request);
		if (!std::equal(_raw.begin() + space + 1, _raw.begin() + line_end, HTTP_VERSION))
			throw protocol_error(http_version_not_supported);
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
		std::string hostname = operator[]("Host");
		if (hostname.empty())
			throw protocol_error(bad_request);
		size_t colon = hostname.find_last_of(':');
		if (colon != std::string::npos)
			try_strtoul(hostname.substr(colon + 1));
		hostname = hostname.substr(0, colon);
		validate_hostname(hostname);
		for (server_pointer_vector::const_iterator server = _socket.get_server_socket().get_servers().begin(); server != _socket.get_server_socket().get_servers().end(); server++)
			for (string_vector::const_iterator name = (*server)->get_names().begin(); name != (*server)->get_names().end(); name++)
			{
				if (*name == hostname)
				{
					_server = *server;
					return ;
				}
			}
	}

	void request::print_request() const
	{
		std::cout << BLUE BOLDED("------- start of received request -------") << CYAN << std::endl;
		write(STDOUT_FILENO, &_raw.front(), _raw.size());
		std::cout << std::endl << BLUE BOLDED("-------- END of received request --------") RESET << std::endl;
	}

	std::string request::operator[](const std::string &header) const
	{
		string_map::const_iterator header_value = _headers.find(uppercase(header));
		if (header_value != _headers.end())
			return (header_value->second);
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

	const std::string &request::get_pathinfo() const
	{
		return (_pathinfo);
	}

	const string_map &request::get_headers() const
	{
		return (_headers);
	}

	const char_vector &request::get_body() const
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

	void request::set_pathinfo(const std::string &pathinfo)
	{
		_pathinfo = pathinfo;
	}

	void request::erase_header(const std::string &header)
	{
		_headers.erase(uppercase(header));
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
			throw protocol_error(bad_request);
		}
	}

	void request::validate_hostname(const std::string &hostname) const
	{
		if (hostname[0] == '.' || hostname[hostname.length() - 1] == '.' || hostname[0] == '-' || hostname[hostname.length() - 1] == '-')
			throw protocol_error(bad_request);
		for (size_t i = 0; i < hostname.length(); i++)
			if (!std::isalnum(hostname[i]))
			{
				if (hostname[i] == '.' || hostname[i] == '-')
				{
					if (i != 0 && !std::isalnum(hostname[i - 1]))
						throw protocol_error(bad_request);
					if (i != hostname.length() - 1 && !std::isalnum(hostname[i + 1]))
						throw protocol_error(bad_request);
				}
				else throw protocol_error(bad_request);
			}
	}
}
