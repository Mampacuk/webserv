#include "request.hpp"

namespace ft
{
	request::~request() {}

	request::request(client_socket socket) : _method(), _uri(), _query(), _pathinfo(), _headers(), _raw(), _body(), _socket(socket), _content_length(-1), _headers_end(std::string::npos), _chunk_size_begin(0), _server()
	{
		_server = _socket.get_server_socket().get_servers().front();
	}

	request::request(const request &other) : _method(other._method), _uri(other._uri), _query(other._query), _pathinfo(other._pathinfo), _headers(other._headers), _raw(other._raw), _body(other._body),
		_socket(other._socket), _content_length(other._content_length), _headers_end(other._headers_end), _chunk_size_begin(other._chunk_size_begin), _server(other._server) {}

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

	std::string request::operator[](const std::string &header) const
	{
		string_map::const_iterator header_value = _headers.find(uppercase(header));
		if (header_value != _headers.end())
			return (header_value->second);
		return ("");
	}

	const string_map &request::get_headers() const
	{
		return (_headers);
	}

	const char_vector &request::get_body() const
	{
		return (_body);
	}

	const client_socket &request::get_socket() const
	{
		return (_socket);
	}

	request::operator int() const
	{
		return (_socket);
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

	// appends chunk to the request. returns whether the request was fully accepted
	bool request::append_chunk(const char *chunk, size_t chunk_size)
	{
		_raw.insert(_raw.end(), chunk, chunk + chunk_size);
		if (_headers_end == std::string::npos)
		{
			if ((_headers_end = search(_raw, std::string(CRLF CRLF))) == std::string::npos)
				return (false);
			const size_t content_length_pos = search(_raw, std::string("Content-Length:"), 0, case_insensitive_equal_to());
			if (content_length_pos != rsearch(_raw, std::string("Content-Length:"), 0, case_insensitive_equal_to()))
				throw protocol_error(bad_request);
			read_header(content_length_pos);
			read_header(search(_raw, std::string("Transfer-Encoding:"), 0, case_insensitive_equal_to()));
			if (operator[]("Content-Length").empty())
			{
				if (operator[]("Transfer-Encoding").empty())
					return (true);
				else if (operator[]("Transfer-Encoding") != "chunked")
					throw protocol_error(bad_request);
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
			size_t chunked = 1; // to enable the while loop
			while (chunked && chunked != std::string::npos)
				chunked = seek_chunk();
			return (chunked == 0);
		}
		return (_raw.size() >= _content_length + _headers_end + std::strlen(CRLF CRLF)
			|| (ends_with(_raw, CRLF) && !ends_with(_raw, CRLF CRLF)));
	}

	void request::parse()
	{
		print_request();
		separate_body();
		size_t pos = parse_request_line();
		_headers_end = std::string::npos; // to allow trailer search
		while (pos != std::string::npos)
			pos = read_header(pos);
		select_server();
		parse_query();
		_raw.clear();
	}

	// should only be called once _headers_end is initialized, i.e. all headers were received
	size_t request::read_header(size_t pos)
	{
		// verify the headers is present, it's not in the body, and it's preceded by a CRLF
		if (pos != std::string::npos && pos < _headers_end && std::equal(_raw.begin() + pos - std::strlen(CRLF), _raw.begin() + pos, CRLF))
		{
			if (std::equal(_raw.begin() + pos, _raw.begin() + pos + std::strlen(CRLF), CRLF))
				return (std::string::npos);
			size_t line_end = search(_raw, std::string(CRLF), pos);
			if (line_end == std::string::npos)
				throw protocol_error(bad_request);
			std::string line = std::string(_raw.begin() + pos, _raw.begin() + line_end);
			if (line.empty())
				return (std::string::npos);
			size_t colon = line.find(':');
			if (colon == 0 || colon == std::string::npos || line.find(' ') < colon)
				throw protocol_error(bad_request);
			std::string key = uppercase(std::string(_raw.begin() + pos, _raw.begin() + pos + colon));
			if (_headers.find(key) != _headers.end())
				throw protocol_error(bad_request);
			size_t val_start = line.find_first_not_of(' ', colon + 1);
			std::string value = (val_start == std::string::npos ? "" : line.substr(val_start));	// val separated (with tail spaces)
			value = value.substr(0, value.find_last_not_of(' ') + 1);							// discard tail spaces
			_headers[key] = value;
			pos = line_end + std::strlen(CRLF);
		}
		return (pos);
	}

	// returns the chunk size. if chunk isn't ready, returns std::string::npos
	// appends chunk to body, throws bad_request if there's chunk size mismatch
	// sets _chunk_size_begin to point to the start of the trailer once 0-chunk is found
	size_t request::seek_chunk()
	{
		if (!_chunk_size_begin)
			_chunk_size_begin = _headers_end + std::strlen(CRLF CRLF);
		const size_t chunk_size_end = search(_raw, std::string(CRLF), _chunk_size_begin);
		if (chunk_size_end == std::string::npos)
			return (std::string::npos);
		const size_t chunk_size = parse_chunk_size(std::string(_raw.begin() + _chunk_size_begin, _raw.begin() + chunk_size_end));
		if (chunk_size == 0)
		{
			_chunk_size_begin = chunk_size_end + std::strlen(CRLF);
			return (chunk_size);
		}
		const size_t chunk_content_begin = chunk_size_end + std::strlen(CRLF);
		const size_t chunk_content_end = search(_raw, std::string(CRLF), chunk_content_begin);
		if (chunk_content_end == std::string::npos)
			return (std::string::npos);
		if (chunk_content_end - chunk_content_begin != chunk_size)
			throw protocol_error(bad_request);
		_body.insert(_body.end(), _raw.begin() + chunk_content_begin, _raw.begin() + chunk_content_end);
		_chunk_size_begin = chunk_content_end + std::strlen(CRLF);
		return (chunk_size);
	}

	size_t	request::parse_chunk_size(const std::string &field) const
	{
		size_t chunk_size;
		std::string field_chunk;

		if (field.empty())
			throw protocol_error(bad_request);
		size_t semicolon = field.find(";");
		chunk_size = try_strtoul(field.substr(0, semicolon), 16);
		field_chunk = field;
		while (semicolon < field_chunk.size() - 1)
		{
			field_chunk = field_chunk.substr(semicolon + 1);
			size_t equal = field_chunk.find("=");
			if (equal < field_chunk.size() - 1)
			{
				validate_chunk_ext_key(field_chunk.substr(0, equal));
				semicolon = field_chunk.find(";", equal + 1);
				validate_chunk_ext_val(field_chunk.substr(equal + 1, (semicolon == std::string::npos ? field_chunk.size() : semicolon) - equal - 1));
			}
			else
				semicolon = std::string::npos;
		}
		if (semicolon == field_chunk.size() - 1)
			throw protocol_error(bad_request);
		return (chunk_size);
	}

	void request::validate_chunk_ext_key(const std::string &token) const
	{
		if (token.empty())
			throw protocol_error(bad_request);
		for (size_t i = 0; i < token.size(); i++)
			if (std::iscntrl(token[i]) || std::string(TOKEN_CHARSET).find(token[i]) != std::string::npos)
				throw protocol_error(bad_request);
	}

	void request::validate_chunk_ext_val(const std::string &ext_val) const
	{
		if (!(ext_val.size() > 1 && ext_val[0] == '"' && ext_val[ext_val.size() - 1] == '"'))
			validate_chunk_ext_key(ext_val);
	}

	void request::print_request() const
	{
		std::cout << CYAN BOLD "------- start of received request -------" RESET CYAN << std::endl;
		write(STDOUT_FILENO, &_raw.front(), _raw.size());
		std::cout << std::endl << CYAN BOLD "-------- end of received request --------" RESET << std::endl;
	}

	void request::separate_body()
	{
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

	void request::parse_query()
	{
		size_t question_mark = _uri.find('?');
		if (question_mark != std::string::npos)
		{
			_query = _uri.substr(question_mark + 1);
			_uri = _uri.substr(0, question_mark);
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
}
