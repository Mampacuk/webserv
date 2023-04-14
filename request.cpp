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
		read_header_value("Transfer-Encoding");
		// check it's not bigger than serv's client_max_body_size
		// if it is, send a "413 Request Entity Too Large" error.
	}

	void request::read_header_value(const std::string &key, int pos)
	{
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
}
