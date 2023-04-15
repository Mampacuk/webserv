#include "request.hpp"

namespace ft
{
	request::~request() {}

	request &request::operator=(const request &other)
	{
		this->_method = other._method;
		this->_uri = other._uri;
		this->_headers = other._headers;
		this->_body = other._body;
		return (*this);
	}

	request::request(int socket) : _method(), _uri(), _headers(), _body(), _raw(), _socket(socket), _content_length(-1), _headers_end(std::string::npos) {}

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
			{
				try
				{
					this->_content_length = parser::strtoul((*this)["Content-Length"]);
				}
				catch (const std::exception &e)
				{
					throw protocol_error("Invalid request received: `Content-Length` error.");
				}
			}
		}
		if (this->_content_length < 0)
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

	const std::string &request::get_body() const
	{
		return (this->_body);
	}

	std::string request::operator[](const std::string &header) const
	{
		string_map::const_iterator header_pair = this->_headers.find(header);
		if (header_pair != this->_headers.end())
			return (header_pair->second);
		return ("");
	}

	int request::operator*() const
	{
		return (this->_socket);
	}

	bool operator<(const request &lhs, const request &rhs)
	{
		return (*lhs < *rhs);
	}

	bool operator==(const request &lhs, const request &rhs)
	{
		return (*lhs == *rhs);
	}
}
