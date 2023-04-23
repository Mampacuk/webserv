#include "response.hpp"

namespace ft
{

	// response::response(): _status(), _body(), _headers(), _req(NULL) {}
	response::response(const response &other): _status(other._status), _body(other._body), _headers(other._headers), _response(other._response), _req(other._req) {}

	response::response(const request &request, int status_code): _status(status_code), _body(), _headers(), _response(other._response), _req(request)
	{
		generate_response();
	}

	response::~response() {}

	response &response::operator=(const response &other)
	{
		this->_status = other._status;
		this->_body = other._body;
		this->_headers = other._headers;

		return (*this);
	}

	response::operator int() const
	{
		return (_req._socket);
	}

	void response::generate_response()
	{
		if (is_error_code(_status))
			find_error_page();

	}

	void find_error_page()
	{

	}

	bool response::is_error_code(int status_code)
	{
		return (status_code >= 400);
	}

	void response::construct_response()
	{
		_response = "HTTP/1.1 ";
		std::stringstream s;
		
		s << _status;

		_response += s.str() + " " + status_to_string(_status) + CRLF;
		for (string_map::const_iterator it = _headers.begin(); it != _headers.end(); it++)
			_response += it->first + ": " + it->second + CRLF;
		_response += CRLF + _body;
	}

	std::string response::status_to_string(int status_code) const
	{
		switch (status_code) {
			case HTTP_STATUS_OK: return "OK";
			case HTTP_STATUS_CREATED: return "Created";
			case HTTP_STATUS_NO_CONTENT: return "No Content";
			case HTTP_STATUS_BAD_REQUEST: return "Bad Request";
			case HTTP_STATUS_FORBIDDEN: return "Forbidden";
			case HTTP_STATUS_NOT_FOUND: return "Not Found";
			case HTTP_STATUS_BAD_METHOD: return "Method Not Allowed";
			case HTTP_STATUS_SERVER_ERROR: return "Internal Server Error";
			case HTTP_STATUS_NOT_SUPPORTED: return "Not Implemented";
			case HTTP_STATUS_SERVICE_UNAVAIL: return "Service Unavailable";
			default: return "Unknown Status Code";
    	}
	}
}