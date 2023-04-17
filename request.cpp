#include "request.hpp"

namespace ft
{
	request::~request() {}

	request::request(const request&) {}

	request &request::operator=(const request &other)
	{
		this->_method = other._method;
		this->_uri = other._uri;
		this->_headers = other._headers;
		return (*this);
	}

	request::request(int socket) : _method(), _uri(), _query(), _headers(), _raw(), _body(), _socket(socket), _content_length(-1), _headers_end(std::string::npos) {}

	// appends chunk to the request. returns whether the request was fully accepted
	bool request::operator+=(const std::string &chunk)
	{
		this->_raw += chunk;
		if (this->_headers_end == std::string::npos)
		{
			if ((this->_headers_end = this->_raw.find(CRLF CRLF)) == std::string::npos)
				return (false);
			read_header(this->_raw.find("Content-Length: "));
			read_header(this->_raw.find("Transfer-Encoding"));
			if (operator[]("Content-Length").empty())
			{
				if (operator[]("Transfer-Encoding").empty())
					return (true);
				else if (operator[]("Transfer-Encoding") != "chunked") // the message is ill-formed
					throw http::protocol_error(400, "Bad Request: Unsupported Transfer Encoding value.");
			}
			else
			{
				if (!operator[]("Transfer-Encoding").empty())
					throw http::protocol_error(400, "Bad Request: Content Length and Transfer Encoding conflict.");
				this->_content_length = try_strtoul(operator[]("Content-Length"));
			}
		}
		if (this->_content_length < 0) // "Transfer-Encoding: chunked" case
			return (ends_with(this->_raw, "0" CRLF CRLF));
		return (this->_raw.size() == this->_content_length + this->_headers_end + std::strlen(CRLF CRLF CRLF));
	}

	// should only be called once _headers_end is initialized, i.e. all headers were received
	size_t request::read_header(size_t pos)
	{
		// verify the headers is present, it's not in the body, and it's preceded by a CRLF
		if (pos != std::string::npos && pos < this->_headers_end && !this->_raw.compare(pos - std::strlen(CRLF), std::strlen(CRLF), CRLF))
		{
			size_t line_end = this->_raw.find(CRLF, pos);
			size_t colon = this->_raw.find(':', pos);
			// check if there's a key, colon is present, and it's followed by one space
			if (colon == pos || colon > line_end || this->_raw[colon + 1] != ' ')
				throw http::protocol_error(400, "Bad Request: Invalid Header.");
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
		while (pos != this->_headers_end)
			pos = read_header(pos);
		// ... check for validity
		parse_query();
		this->_raw.clear();
	}

	void request::separate_body()
	{
		if (operator[]("Transfer-Encoding").empty())
			this->_body += this->_raw.substr(this->_headers_end + std::strlen(CRLF CRLF), this->_content_length);
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
					throw http::protocol_error(400, "Bad Request: Chunk Size Mismatch.");
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
		if (space > line_end)
			throw http::protocol_error(HTTP_STATUS_BAD_REQUEST, "Bad Request: Method unspecified.");
		this->_method = this->_raw.substr(0, space);
		space = this->_raw.find(' ', space + 1);
		if (space > line_end)
			throw http::protocol_error(HTTP_STATUS_BAD_REQUEST, "Bad Request: URI unspecified.");
		this->_uri = this->_raw.substr(this->_method.length() + 1, space - this->_method.length() - 1);
		if (this->_raw.compare(space + 1, line_end - space - 1, "HTTP/1.1"))
			throw http::protocol_error(HTTP_STATUS_VERSION_NOT_SUP, "HTTP Version Not Supported: Invalid Protocol Version.");
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
			throw http::protocol_error(HTTP_STATUS_BAD_REQUEST, "Bad Request: Unsigned integer parsing failed.");
		}
	}
}
