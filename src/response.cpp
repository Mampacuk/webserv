#include "../include/response.hpp"

namespace ft
{
	response::response() : _status(), _body(), _headers(), _message(), _request(), _cursor(), _location() {}
	
	response::response(const response &other) : _status(other._status), _body(other._body), _headers(other._headers), _message(other._message), _request(other._request), _cursor(other._cursor), _location(other._location) {}

	response::response(const request &request, http::code status) : _status(status), _body(), _headers(), _message(), _request(request), _cursor(), _location()
	{
		generate_response();
	}

	response::~response()
	{
		close(this->_request);
	}

	response &response::operator=(const response &other)
	{
		this->_status = other._status;
		this->_body = other._body;
		this->_headers = other._headers;
		this->_message = other._message;
		this->_location = other._location;
		return (*this);
	}

	response::operator int() const
	{
		return (this->_request);
	}

	void response::generate_response()
	{
		if (http::is_error_code(this->_status))
			find_error_page();
		else
		{
			if (this->_request.get_method() == "GET")
				get();

		}
		//For get method
		//read_requested_file(); // if not found again the error_page part into the body
			// if (is_error_code(_status))
			// 	find_error_page();
			// else
			// 	find_requested_uri();

	}

	void response::get()
	{
		read_requested_file();
	}

	void response::read_requested_file()
	{
		
	}

	void response::post()
	{

	}

	// void find_error_page()
	// {

	// }

	void response::find_location()
	{
		for (location_set::const_iterator loc = _request.get_server().get_locations().begin(); 
						loc != _request.get_server().get_locations().end(); loc++)
		{
			if (starts_with(_request._uri, loc->get_route()))
			{
				if (loc->has_modifier() && loc->get_route() == _request._uri)
				{
					_location = &(*loc);
					break;
				}
				if (_location == NULL || _location->get_route().length() < loc->get_route().length())
					_location = &(*loc);
			}
		}
		if (_location == NULL)
			throw server::server_error(404, "Not found");
	}

	void response::construct_response()
	{
		std::stringstream s;

		this->_message = "HTTP/1.1 ";
		s << this->_status;
		this->_message += s.str() + " " + http::reason_phrase(this->_status) + CRLF;
		for (string_map::const_iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
			this->_message += it->first + ": " + it->second + CRLF;
		this->_message += CRLF + _body;
	}


	std::string response::get_chunk()
	{
		std::string chunk = this->_message.substr(this->_cursor, BUFSIZ);
		this->_cursor = std::min(this->_cursor + BUFSIZ - 1, this->_message.size());
		return (chunk);
	}

	bool response::empty() const
	{
		return (this->_message.empty());
	}

	bool response::sent() const
	{
		return (this->_cursor == this->_message.size());
	}
}
