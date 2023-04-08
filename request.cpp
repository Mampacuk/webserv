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

	// first parse Content-Length and see if it's completed
	request::request(const std::string &message) : _raw(message)
	{
		read_header_value("Content-Length");
		// check it's not bigger than serv's client_max_body_size
		// check it's valid
	}

	void request::read_header_value(const std::string &key, int pos)
	{
		if (pos == std::string::npos)
			pos = this->_raw.find(key + ": ");
		// verify the headers is present and it's preceded by a CLRF
		if (pos != std::string::npos && this->_raw.substr(pos - 2, std::strlen(CLRF)) == CLRF)
		{
			size_t start = this->_raw.find_first_not_of(' ', pos + key.length() + 2);	// line start
			size_t end = this->_raw.find(CLRF, pos + key.length() + 2) - 1;				// line end
			std::string value = this->_raw.substr(start, end - start + 1);				// line separated
			value = value.substr(0, value.find_last_not_of(' ') + 1);					// discard tail spaces
			this->_headers[key] = value;
		}
	}

	const std::string &request::get_body() const
	{
		return (this->_body);
	}

	request::operator bool()
	{
		return (this->_headers.find("Content-Length") != this->_headers.end());
	}
	// const std::string &request::get_header_value(const std::string &header) const
	// {
	// 	return (this->_headers[header]);
	// }
}
