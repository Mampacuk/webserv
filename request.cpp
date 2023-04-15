#include "request.hpp"

namespace ft
{
	request::~request() {}

	request &request::operator=(const request &other)
	{
		this->_method = other._method;
		this->_uri = other._uri;
		this->_headers = other._headers;
		return (*this);
	}

	request::request(int socket) : _method(), _uri(), _headers(), _raw(), _socket(socket), _content_length(-1), _headers_end(std::string::npos) {}

	// appends chunk to the request. returns whether the request was fully accepted
	bool request::operator+=(const std::string &chunk)
	{
		this->_raw += chunk;
		if (this->_headers_end == std::string::npos)
		{
			if ((this->_headers_end = this->_raw.find(CLRF CLRF)) == std::string::npos)
				return (false);
			read_header_value("Content-Length");
			read_header_value("Transfer-Encoding");
			if ((*this)["Content-Length"].empty())
			{
				if ((*this)["Transfer-Encoding"].empty())
					return (true);
				else if ((*this)["Transfer-Encoding"] != "chunked") // the message is ill-formed
					throw protocol_error("Invalid request received: `Transfer-Encoding` error.");
			}
			else
				this->_content_length = parser::strtoul((*this)["Content-Length"]);
		}
		if (this->_content_length < 0) // "Transfer-Encoding: chunked" case
			return (ends_with(this->_raw, "0" CLRF CLRF));
		return (this->_raw.size() == this->_content_length + this->_headers_end + std::strlen(CLRF CLRF CLRF));
	}

	void request::read_header_value(const std::string &key, int pos)
	{
		if (this->_headers.find(key) != this->_headers.end())
			return ;
		if (pos == std::string::npos)
			pos = this->_raw.rfind(key + ": "); // last value is saved
		/*
		** if we find headers with multi-values, this should run a loop
		** and combine all repeated values into a comma-separated list
		*/
		// verify the headers is present and it's preceded by a CLRF
		if (pos != std::string::npos && this->_raw.substr(pos - std::strlen(CLRF), std::strlen(CLRF)) == CLRF)
		{
			size_t start = this->_raw.find_first_not_of(' ', pos + key.length() + 2);	// line start; 2 is to skip ": "
			size_t end = this->_raw.find(CLRF, start);									// line end; 2 is to skip ": "
			std::string value = this->_raw.substr(start, end - start);					// line separated
			value = value.substr(0, value.find_last_not_of(' ') + 1);					// discard tail spaces
			this->_headers[key] = value;
		}
	}

	void request::decode_chunked_transfer()
	{
		if ((*this)["Transfer-Encoding"].empty())
			return ;
		std::string body;
		this->_content_length = 0;
		int pos = this->_headers_end + std::strlen(CLRF CLRF);
		int end_of_line = this->_raw.find(CLRF, pos);
		int chunk_size = parser::strtoul(this->_raw.substr(pos, end_of_line - pos), 16);
		while (chunk_size > 0)
		{
			pos = end_of_line + std::strlen(CLRF);		// now points to the beginning of chunk
			end_of_line = this->_raw.find(CLRF, pos);	// now points to the end of chunk
			if (end_of_line - pos != chunk_size)
				throw protocol_error("Bad request: chunk size mismatch.");
			body += this->_raw.substr(pos, chunk_size);	// append the chunk
			this->_content_length += chunk_size;
			pos = end_of_line + std::strlen(CLRF);		// now points to the beginning of chunk-size
			end_of_line = this->_raw.find(CLRF, pos);	// now points to the end of chunk-size
			chunk_size = parser::strtoul(this->_raw.substr(pos, end_of_line - pos), 16);
			// 45\r\nbla\r\n0\r\n
			// 012 3 4567 9
		}
		//...do something with body... wanna sleep... z-z-z-z...
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
}
