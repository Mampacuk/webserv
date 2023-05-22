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
		this->_server = other._server;
		return (*this);
	}

	request::request(client_socket socket) : _method(), _uri(), _query(), _headers(), _raw(), _body(), _socket(socket), _content_length(-1), _headers_end(std::string::npos), _server() {}

	request::request(const request &other) : _method(other._method), _uri(other._uri), _query(other._query), _headers(other._headers), _raw(other._raw), _body(other._body),
		_socket(other._socket), _content_length(other._content_length), _headers_end(other._headers_end), _server(other._server) {}

	// appends chunk to the request. returns whether the request was fully accepted
	bool request::operator+=(const std::string &chunk)
	{
		// std::cout << RED "vvvvvvv received chunk of size " << chunk.size() << "vvvvvvv" RESET << std::endl;
		// std::cout << chunk << std::endl;
		// std::cout << RED "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" RESET << std::endl;
		
		this->_raw += chunk;
		if (this->_headers_end == std::string::npos)
		{
			if ((this->_headers_end = this->_raw.find(CRLF CRLF)) == std::string::npos)
			{
				std::cout << CYAN "NOT DONE RECEIVING HEADERS" RESET << std::endl; 
				return (false);
			}
			read_header(this->_raw.find("Content-Length: "));
			read_header(this->_raw.find("Transfer-Encoding: "));
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
				this->_content_length = try_strtoul(operator[]("Content-Length"));
			}
		}
		if (this->_content_length < 0) // "Transfer-Encoding: chunked" case
		{
			std::cout << CYAN BOLDED("TRANSFER ENCODING CASE") RESET << std::endl;
			return (ends_with(this->_raw, "0" CRLF CRLF));
		}
		std::cout << CYAN "COMPELTED RECEIVING REQUEST? " << ((this->_raw.size() == this->_content_length + this->_headers_end + std::strlen(CRLF CRLF CRLF)
			|| (ends_with(this->_raw, CRLF) && !ends_with(this->_raw, CRLF CRLF))) ? "yes" : "no") << RESET << std::endl;
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
				throw protocol_error(bad_request, "Invalid Header.");
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
		std::cout << "received request is" << std::endl << LRED << this->_raw << RESET << std::endl;
		separate_body();
		size_t pos = parse_request_line();
		// std::cout << "method: |" << this->_method << "|" << std::endl;
		// std::cout << "content length: " << this->_content_length << std::endl;
		// std::cout << "headers end: " << this->_headers_end << std::endl;
		// std::cout << "body: |" << this->_body << "|, empty? " << (this->_body.empty() ? "yes" : "no") << std::endl;
		while (pos != this->_headers_end + std::strlen(CRLF))
			pos = read_header(pos);
		// ... check for validity
		parse_query();
		// std::cout << CYAN "SELECTED SERVER?" RESET << std::endl;
		select_server();
		// std::cout << CYAN BOLDED("YES") RESET << std::endl;
		
		// std::cout << BLUE;
		// for (string_map::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
		// 	std::cout << "header: { " << it->first << " : " << it->second << " }" << std::endl;
		// std::cout << "uri: |" << this->_uri << "|" << std::endl;
		// std::cout << "query: |" << this->_query << "|" RESET << std::endl;
		this->_raw.clear();
	}

	void request::separate_body()
	{
		if (operator[]("Transfer-Encoding").empty())
		{
			const size_t read_body_start = this->_headers_end + std::strlen(CRLF CRLF);
			const size_t read_body_end = this->_raw.size() - (ends_with(this->_raw, CRLF) && !ends_with(this->_raw, CRLF CRLF) ? std::strlen(CRLF) : 0);
			this->_body += this->_raw.substr(read_body_start, read_body_end - read_body_start);
			if (this->_content_length == -1)
				this->_content_length = this->_body.size();
			else if (static_cast<size_t>(this->_content_length) != read_body_end - read_body_start)
				throw protocol_error(bad_request, "Content-Length mismatch.");
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
					throw protocol_error(bad_request, "Chunk Size Mismatch.");
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
			throw protocol_error(bad_request, "Method unspecified.");
		this->_method = this->_raw.substr(0, space);
		space = this->_raw.find(' ', space + 1);
		if (space > line_end)
			throw protocol_error(bad_request, "URI unspecified.");
		this->_uri = this->_raw.substr(this->_method.length() + 1, space - this->_method.length() - 1);
		if (this->_raw.compare(space + 1, line_end - space - 1, HTTP_VERSION))
			throw protocol_error(http_version_not_supported, "Invalid Protocol Version.");
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

	void request::select_server()
	{
		const std::string hostname = operator[]("Host");
		if (hostname.empty())
			throw protocol_error(bad_request, "Host unspecified.");
		for (server_pointer_vector::const_iterator server = this->_socket.get_server_socket().get_servers().begin(); server != this->_socket.get_server_socket().get_servers().end(); server++)
			for (string_vector::const_iterator name = (*server)->get_names().begin(); name != (*server)->get_names().end(); name++)
			{
				if (*name == hostname)
				{
					this->_server = *server;
					return ;
				}
			}
		this->_server = this->_socket.get_server_socket().get_servers().front();
	}

	std::string request::operator[](const std::string &header) const
	{
		string_map::const_iterator header_pair = this->_headers.find(header);
		if (header_pair != this->_headers.end())
			return (header_pair->second);
		return ("");
	}

	const client_socket &request::get_socket() const
	{
		return (this->_socket);
	}

	const std::string &request::get_method() const
	{
		return (this->_method);
	}

	const std::string &request::get_uri() const
	{
		return (this->_uri);
	}

	const std::string &request::get_query() const
	{
		return (this->_query);
	}

	const std::string &request::get_body() const
	{
		return (this->_body);
	}

	ssize_t request::get_content_length() const
	{
		return (this->_content_length);
	}

	const server &request::get_server() const
	{
		if (!this->_server)
			throw std::runtime_error("Querying an incomplete request for its server is not allowed.");
		return (*this->_server);
	}

	request::operator int() const
	{
		return (this->_socket);
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
