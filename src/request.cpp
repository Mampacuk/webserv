#include "request.hpp"

namespace ft
{
	request::~request() {}

	request &request::operator=(const request &other)
	{
		this->_method = other._method;
		this->_uri = other._uri;
		this->_query = other._query;
		this->_headers = other._headers;
		this->_raw = other._raw;
		this->_body = other._body;
		this->_socket = other._socket;
		this->_content_length = other._content_length;
		this->_headers_end = other._headers_end;
		return (*this);
	}

	request::request(int socket) : _method(), _uri(), _query(), _headers(), _raw(), _body(), _socket(socket), _content_length(-1), _headers_end(std::string::npos) {}

	request::request(const request &other) : _method(other._method), _uri(other._uri), _query(other._query), _headers(other._headers), _raw(other._raw), _body(other._body),
		_socket(other._socket), _content_length(other._content_length), _headers_end(other._headers_end) {}

	// appends chunk to the request. returns whether the request was fully accepted
	bool request::operator+=(const std::string &chunk)
	{
		std::cout << "appending chunk |" << chunk << "| of size " << chunk.size() << std::endl;
		this->_raw += chunk;
		if (this->_headers_end == std::string::npos)
		{
			if ((this->_headers_end = this->_raw.find(CRLF CRLF)) == std::string::npos)
				return (false);
			std::cout << "hit headers_end: it's now " << this->_headers_end << std::endl;
			std::cout << "this->_raw.find(\"Content-Length: \") points to |" << this->_raw[this->_raw.find("Content-Length: ")] << "|" << std::endl;
			std::cout << "this->_raw.find(\"Transfer-Encoding: \") points to |" << this->_raw[this->_raw.find("Transfer-Encoding: ")] << "|" << std::endl;
			read_header(this->_raw.find("Content-Length: "));
			read_header(this->_raw.find("Transfer-Encoding: "));
			std::cout << "Content-Length value: |" << operator[]("Content-Length") << "|" << std::endl; 
			std::cout << "Transfer-Encoding value: |" << operator[]("Transfer-Encoding") << "|" << std::endl; 
			if (operator[]("Content-Length").empty())
			{
				if (operator[]("Transfer-Encoding").empty())
					return (true);
				else if (operator[]("Transfer-Encoding") != "chunked") // the message is ill-formed
					throw http::protocol_error(http::code::bad_request, "Bad Request: Unsupported Transfer Encoding value.");
			}
			else
			{
				if (!operator[]("Transfer-Encoding").empty())
					throw http::protocol_error(http::code::bad_request, "Bad Request: Content Length and Transfer Encoding conflict.");
				this->_content_length = try_strtoul(operator[]("Content-Length"));
			}
		}
		if (this->_content_length < 0) // "Transfer-Encoding: chunked" case
			return (ends_with(this->_raw, "0" CRLF CRLF));
		return (this->_raw.size() == this->_content_length + this->_headers_end + std::strlen(CRLF CRLF CRLF)
			|| (ends_with(this->_raw, CRLF) && !ends_with(this->_raw, CRLF CRLF)));
	}

	// should only be called once _headers_end is initialized, i.e. all headers were received
	size_t request::read_header(size_t pos)
	{
		// verify the headers is present, it's not in the body, and it's preceded by a CRLF
		if (pos != std::string::npos && pos < this->_headers_end && !this->_raw.compare(pos - std::strlen(CRLF), std::strlen(CRLF), CRLF))
		{
			size_t line_end = this->_raw.find(CRLF, pos);
			size_t colon = this->_raw.find(':', pos);
			// check if there's a key, colon is present, there's no spaces in header and it's followed by one space
			if (colon == pos || colon > line_end || colon != this->_raw.find(' ', pos) - 1)
				throw http::protocol_error(http::code::bad_request, "Bad Request: Invalid Header.");
			std::string key = this->_raw.substr(pos, colon - pos);
			size_t val_start = this->_raw.find_first_not_of(' ', colon + 2);		// val start; 2 is to skip ": "
			std::string value = this->_raw.substr(val_start, line_end - val_start);	// val separated (with tail spaces)
			value = value.substr(0, value.find_last_not_of(' ') + 1);				// discard tail spaces
			this->_headers[key] = value;
			pos = line_end + std::strlen(CRLF);
		}
		return (pos);
	}

	void request::parse()
	{
		separate_body();
		size_t pos = parse_request_line();
		std::cout << "method: |" << this->_method << "|" << std::endl;
		std::cout << "conetent length: " << this->_content_length << std::endl;
		std::cout << "headers end: " << this->_headers_end << std::endl;
		std::cout << "body: |" << this->_body << "|, empty? " << (this->_body.empty() ? "yes" : "no") << std::endl;
		while (pos != this->_headers_end + std::strlen(CRLF))
		{
			pos = read_header(pos);
			// if (pos >= this->_raw.size())
			// {
			// 	std::cout << "broke out of while with pos=" << pos << std::endl;
			// 	break ;
			// }
		}
		// ... check for validity
		parse_query();
		for (string_map::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
			std::cout << "header: { " << it->first << " : " << it->second << " }" << std::endl;
		std::cout << "uri: |" << this->_uri << "|" << std::endl;
		std::cout << "query: |" << this->_query << "|" << std::endl;
		this->_raw.clear();
	}

	void request::separate_body()
	{
		if (operator[]("Transfer-Encoding").empty())
		{
			const size_t read_body_start = this->_headers_end + std::strlen(CRLF CRLF);
			const size_t read_body_end = this->_raw.size() - (ends_with(this->_raw, CRLF) && !ends_with(this->_raw, CRLF CRLF) ? std::strlen(CRLF) : 0);
			std::cout << "read_body_start:" << read_body_start << ", read_body_end:" << read_body_end << std::endl;
			std::cout << "read_body_end - read_body_start:" << read_body_end - read_body_start << std::endl;
			this->_body += this->_raw.substr(read_body_start, read_body_end - read_body_start);
			std::cout << "body: |" << this->_body << "|, empty? " << (this->_body.empty() ? "yes" : "no") << std::endl;
			std::cout << "content-length: " << this->_content_length << std::endl;
			if (this->_content_length == -1)
				this->_content_length = this->_body.size();
			else if (static_cast<size_t>(this->_content_length) != read_body_end - read_body_start)
				throw http::protocol_error(http::code::bad_request, "Bad Request: Content-Length mismatch.");
		}
		else
		{
			size_t	chunk_size;
			size_t	pos = this->_headers_end + std::strlen(CRLF CRLF);
			size_t	end_of_line = this->_raw.find(CRLF, pos);
			chunk_size = try_strtoul(this->_raw.substr(pos, end_of_line - pos), 16);
			this->_content_length = 0;
			while (chunk_size > 0)
			{
				pos = end_of_line + std::strlen(CRLF);		// now points to the beginning of chunk
				end_of_line = this->_raw.find(CRLF, pos);	// now points to the end of chunk
				if (end_of_line - pos != chunk_size)
					throw http::protocol_error(http::code::bad_request, "Bad Request: Chunk Size Mismatch.");
				this->_body += this->_raw.substr(pos, chunk_size);	// append the chunk to the body
				this->_content_length += chunk_size;
				pos = end_of_line + std::strlen(CRLF);		// now points to the beginning of chunk-size
				end_of_line = this->_raw.find(CRLF, pos);	// now points to the end of chunk-size
				chunk_size = try_strtoul(this->_raw.substr(pos, end_of_line - pos), 16);
			}
		}
		this->_raw.erase(this->_headers_end + std::strlen(CRLF CRLF));
	}

	size_t request::parse_request_line()
	{
		size_t line_end = this->_raw.find(CRLF);
		size_t space = this->_raw.find(' ');
		if (!space || space > line_end)
			throw http::protocol_error(http::code::bad_request, "Bad Request: Method unspecified.");
		this->_method = this->_raw.substr(0, space);
		space = this->_raw.find(' ', space + 1);
		if (space > line_end)
			throw http::protocol_error(http::code::bad_request, "Bad Request: URI unspecified.");
		this->_uri = this->_raw.substr(this->_method.length() + 1, space - this->_method.length() - 1);
		if (this->_raw.compare(space + 1, line_end - space - 1, "HTTP/1.1"))
			throw http::protocol_error(http::code::http_version_not_supported, "HTTP Version Not Supported: Invalid Protocol Version.");
		return (line_end + std::strlen(CRLF));
	}

	void request::parse_query()
	{
		size_t question_mark = this->_uri.find('?');
		if (question_mark != std::string::npos)
		{
			this->_query = this->_uri.substr(question_mark + 1);
			this->_uri = this->_uri.substr(question_mark);
		}
	}

	std::string request::operator[](const std::string &header) const
	{
		string_map::const_iterator header_pair = this->_headers.find(header);
		if (header_pair != this->_headers.end())
			return (header_pair->second);
		return ("");
	}

	int request::get_socket() const
	{
		return (this->_socket);
	}

	const std::string &request::get_method() const
	{
		return (this->_method);
	}

	request::operator int() const
	{
		return (this->_socket);
	}

	unsigned int request::try_strtoul(const std::string &number, int base) const
	{
		try
		{
			return (parser::strtoul(number, base));
		}
		catch (const std::exception &e)
		{
			throw http::protocol_error(http::code::bad_request, "Bad Request: Unsigned integer parsing failed.");
		}
	}
}
