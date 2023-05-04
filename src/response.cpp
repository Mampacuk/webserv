#include "../include/response.hpp"		//change later

namespace ft
{
	response::response(const response &other) : _status(other._status), _body(other._body), _headers(other._headers), _message(other._message), _uri(other._uri), _request(other._request), _location(other._location), _cursor(other._cursor), _path(other._path) {}

	response::response(const request &request, http::code status) : _status(status), _body(), _headers(), _message(), _uri(request.get_uri()), _request(request), _location(), _cursor(), _path()
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
			find_error_page();	//shoudl become like the catch block
		else
		{
			try
			{
				find_rewritten_location();
				if (!_location->method_allowed(_request.get_method()))
					throw server::server_error(405, "Method not allowed.");
				_path = _location->get_root() + _uri;
				if (this->_request.get_method() == "GET")
					get();                                    
				else if (this->_request.get_method() == "POST")
					post();
				// else if ... delete() else throw exception ("invalid method")
			}
			catch(server::server_error e)
			{
				// if (!read_requested_file(_location->get_error_page(e)))
				// 	if (!read_requested_file(_location->get_error_page(404)))
						//construct somehow a 404
				//add_headers();
				//construct response();
			}
		}
	}

	void response::get()
	{
		find_requested_file();
		//add_headers();
		//construct_response();

	}

	bool response::read_requested_file(const std::string &path)
	{
		std::ifstream file;

		file.open(path);
		if (file.is_open())
		{
			_path = path;
			file >> _body;
			// content length and body size should be the same I guess
			return (true);
		}
		return (false);
	}

	void response::find_requested_file()
	{
		if (ends_with(_path, "/"))
		{
			if (!_location->get_indexes().empty())
			{
				for (ft::string_vector::const_iterator it = _location->get_indexes().begin(); it != _location->get_indexes().end(); it++)
					if (read_requested_file(_path + *it))
						break;
			}
			else
			{
				if (_location->get_autoindex())
				{
					//somewhow generate the response;
				}
				else
					throw server::server_error(404, "File not found.");
			}
		}
		else
			if (!read_requested_file(_path))
				throw server::server_error(404, "File not found.");
	}

	void response::post()
	{
		const std::string cgi = this->_location->get_cgi_param("SCRIPT_FILENAME");
		if (cgi.empty())
			throw server::server_error(http::code::internal_server_error, "POST method with unspecified CGI is not allowed.");
		
		// if execve() failed return 404 file not found
	}

	void response::find_error_page()
	{

	}

	void response::find_location()
	{
		for (location_set::const_iterator loc = _request.get_server().get_locations().begin(); 
						loc != _request.get_server().get_locations().end(); loc++)
		{
			if (starts_with(_uri, loc->get_route()))
			{
				if (loc->has_modifier() && loc->get_route() == _uri)
				{
					_location = &(*loc);
					break;
				}
				if (_location == NULL || _location->get_route().length() < loc->get_route().length())
					_location = &(*loc);
			}
		}
		if (_location == NULL)
			throw server::server_error(404, "File not found.");
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

	bool response::rewrite(const std::string &portion, const std::string &withwhat)
	{
		size_t pos = _uri.find(portion);
		if (pos)
		{
			_uri.replace(pos, pos + portion.length(), portion);
			return true;
		}
		return false;
	}

	void response::find_rewritten_location()
	{
		if (!_request.get_server().get_redirects().empty())
			for (ft::string_mmap::const_iterator it = _request.get_server().get_redirects().begin(); it != _request.get_server().get_redirects().end(); it++)
				rewrite(it->first, it->second);
		int i = 0;
		find_location();
		while (i != 10)
		{
			if (!_location->get_redirects().empty())
			{
				for (ft::string_mmap::const_iterator it = _location->get_redirects().begin(); it != _location->get_redirects().end(); it++)
					rewrite(it->first, it->second);
				find_location();
				i++;
			}
			else
				break;
		}
	}
}
