#include "response.hpp"

namespace ft
{
	response::response(const response &other) : _request(other._request), _status(other._status), _headers(other._headers), _uri(other._uri), _path(other._path), _body(other._body), _message(other._message), _cursor(other._cursor), _cgi(other._cgi), _location(other._location) {}

	response::response(const request &request, http_code status) : _request(request), _status(status), _headers(), _uri(request.get_uri()), _path(), _body(), _message(), _cursor(), _cgi(), _location()
	{
		generate_response();
	}

	response::~response() {}

	response &response::operator=(const response &other)
	{
		_request = other._request;
		_status = other._status;
		_headers = other._headers;
		_uri = other._uri;
		_path = other._path;
		_body = other._body;
		_message = other._message;
		_cursor = other._cursor;
		_location = other._location;
		_cgi = other._cgi;
		return (*this);
	}

	response::operator int() const
	{
		return (_request);
	}

	void response::generate_response()
	{
		try
		{
			if (http::is_error_code(_status))
				throw server_error(_status);
			find_rewritten_location();
			_path = append_trailing_slash(_location->get_root() + _uri);
			_cgi = _location->get_cgi_executable(get_file_extension(_uri));
			if (_request.get_method() == "GET")
			{
				if (!_cgi.empty())
					post_method(_cgi);
				else get_method();
			}
			else if (_request.get_method() == "POST")
				post_method(_cgi);
			else if (_request.get_method() == "DELETE")
				delete_method();
			else throw server_error(not_implemented);
		}
		catch (const server_error &e)
		{
			_status = e;

			if (_location)
			{
				std::string prev_uri = _uri;
				if (!_location->get_error_page(e).empty())
					_uri = _location->get_route() + _location->get_error_page(e);
				const location *loc = _location;
				find_location(_request.get_server());
				if (loc != _location)
				{
					_status = ok;
					generate_response();
					return ;
				}
				else
					_uri = prev_uri;
			}
			if (!_location)
				read_error_page(e);
			else if (!read_requested_file(_location->get_root() + _location->get_route() + _location->get_error_page(e)))
				if (!read_requested_file(_location->get_root() + _location->get_route() + _location->get_error_page(not_found)))
					read_error_page(e);
		}
		construct_response();
	}

	void response::construct_error_page(http_code error)
	{
		std::string content = "<html>\n\t<head>\n\t\t<title>Error " + to_string(error) + "</title>\n\t</head>"
				+ "\n\t<body>\n\t\t<h1>Error " + to_string(error) + " " + reason_phrase(_status) + "</h1>\n\t</body>\n</html>\n";
		_body.insert(_body.end(), content.begin(), content.end());
	}

	void response::read_error_page(http_code error) //check how the path is constructed
	{
		std::string error_page;
		
		if (!_location)
			error_page = _request.get_server().get_root() + _request.get_server().get_error_page(error);
		else
			error_page = _location->get_root() + _location->get_route() + _location->get_error_page(error);
		if (error_page.empty() || !read_requested_file(error_page))
			construct_error_page(error);
	}

	void response::get_method()
	{
		find_requested_file();
	}

	bool response::read_requested_file(const std::string &filename)
	{
		std::ifstream file;

		// std::cout << "Error code: " << e << std::endl;
		// std::cout << "Error page: " << filename << std::endl;
		if (!is_regular_file(filename.c_str()))
			return (false);
		file.open(filename);
		if (file)
		{
			std::streampos file_len;
			std::ostringstream ss;

			_path = filename;
			file.seekg(0, std::ios::end);
			file_len = file.tellg();
			file.seekg(0, std::ios::beg);
			_body.reserve(file_len);
			ss << file.rdbuf();
			std::string buffer(ss.str().c_str(), file_len);
			_body.insert(_body.end(), buffer.begin(), buffer.end());
			if (_body.size() != 0)
			{
				std::string ext = get_file_extension(_path);
				if (ext == "html")
					_headers["Content-Type"] = "text/html";
				else if (ext == "txt")
					_headers["Content-Type"] = "text/plain";
				else if (ext == "css")
					_headers["Content-Type"] = "text/css";
				else if (ext == "js")
					_headers["Content-Type"] = "text/javascript";
				else if (ext == "jpg" || ext == "jpeg")
					_headers["Content-Type"] = "image/jpeg";
			}
			return (true);
		}
		return (false);
	}

	void response::find_requested_file()
	{
		if (ends_with(_path, "/"))
		{
			ft::string_vector::const_iterator it = _location->get_indices().begin();
			for (; it != _location->get_indices().end(); it++)
			{
				if (file_exists(_path + *it))
				{
					std::string prev_uri = _uri;
					_uri = (ends_with(_uri, "/") ? _uri : (_uri + "/")) + *it;
					const location *loc = _location; //Alexxx help
					find_location(_request.get_server());
					if (loc != _location || is_regular_file((_path + *it).c_str()))
					{
						generate_response();
						return ;
					}
					else
						_uri = prev_uri;
				}
			}
			if (it == _location->get_indices().end())
			{
				if (_location->get_autoindex())
					generate_autoindex(_path);
				else throw server_error(not_found);		//forbidden
			}
		}
		else
			if (!read_requested_file(_path))
				throw server_error(not_found);
	}

	bool response::file_exists(const std::string &filename)
	{
		std::ifstream file;
		DIR *directory = opendir(filename.c_str());
		
		if (directory)
		{
			closedir(directory);
			return (true);
		}
		if (!is_regular_file(filename.c_str()))
			return (false);
		file.open(filename);
		return (file ? true : false);
	}

	void response::find_location(const base_dir_ext &level)
	{
		location_set::const_iterator loc = level.get_locations().begin();
		for (; loc != level.get_locations().end(); loc++)
		{
			if (starts_with(_uri, loc->get_route()))
			{
				if (loc->has_modifier() && loc->get_route() == _uri)
				{
					_location = &(*loc);
					break;
				}
				if ((_location == NULL || _location->get_route().length() < loc->get_route().length()) && !loc->has_modifier())
				{
					_location = &(*loc);
					find_location(*_location);
				}
			}
		}
		if (_location == NULL)
		{
			throw server_error(not_found);
		}
	}

	void response::construct_response()
	{
		_headers["Content-Length"] = to_string(_body.size());
		if (!_cgi.empty() && _status == ok)
			_status = im_used;
		std::string buffer = HTTP_VERSION " " + to_string(_status) + " " + reason_phrase(_status) + CRLF;
		_message.insert(_message.end(), buffer.begin(), buffer.end());
		for (string_map::const_iterator it = _headers.begin(); it != _headers.end(); it++)
		{
			buffer = it->first + ": " + it->second + CRLF;
			_message.insert(_message.end(), buffer.begin(), buffer.end());
		}
		if (_cgi.empty())
		{
			buffer = CRLF;
			_message.insert(_message.end(), buffer.begin(), buffer.end());
		}
		_message.insert(_message.end(), _body.begin(), _body.end());
		_body.clear();
		print_response();
	}

	char_vector_iterator_pair response::get_chunk()
	{
		char_vector::iterator begin = _message.begin() + _cursor;
		_cursor = std::min(_cursor + BUFSIZ, _message.size());
		char_vector::iterator end = _message.begin() + _cursor;
		return (std::make_pair(begin, end));
	}

	bool response::empty() const
	{
		return (_message.empty());
	}

	bool response::sent() const
	{
		return (_cursor == _message.size());
	}

	bool response::rewrite(const std::string &what, const std::string &with_what)
	{
		size_t pos = _uri.find(what);
		if (pos != std::string::npos)
		{
			_uri.replace(pos, pos + what.length(), with_what);
			_status = found;
			return true;
		}
		return false;
	}

	void response::find_rewritten_location()
	{
		for (string_map::const_iterator it = _request.get_server().get_rewrites().begin(); it != _request.get_server().get_rewrites().end(); it++)
			rewrite(it->first, it->second);
		int i = 0;
		find_location(_request.get_server());
		while (i != 10)
		{
			if (!_location->get_rewrites().empty())
			{
				for (string_map::const_iterator it = _location->get_rewrites().begin(); it != _location->get_rewrites().end(); it++)
					rewrite(it->first, it->second);
				find_location(_request.get_server());
				i++;
			}
			else
				break;
		}
		if (!_location->is_allowed_method(_request.get_method()))
			throw server_error(method_not_allowed);
		if (_location->get_client_max_body_size() < _request.get_body().size())
			throw server_error(content_too_large);
		if (_request.get_uri().length() + (_request.get_query().empty() ? 0 : _request.get_query().length() + 1) >= MAX_URI_LENGTH)
			throw server_error(uri_too_long);
		parse_pathinfo();
	}

	void response::parse_pathinfo()
	{
		size_t extension = std::string::npos;
		for (string_map::const_iterator it = _location->get_cgi().begin(); it != _location->get_cgi().end(); it++)
		{
			size_t tmp = _uri.find("." + it->first);
			if (tmp != std::string::npos && tmp <= extension && (tmp + it->first.size() + 1 == _uri.size() || _uri[tmp + it->first.size() + 1] == '/'))
					extension = tmp + it->first.size() + 1;
		}
		if (extension != std::string::npos)
			_request.set_pathinfo(_uri.substr(extension));
		_uri = _uri.substr(0, extension);
	}

	void response::generate_autoindex(const std::string &path) 
	{
		DIR *dir = opendir(path.c_str());
		string_vector files;
		std::string buffer;

		if (dir == NULL)
			throw server_error(not_found);
		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
			if (entry->d_name[0] != '.')
				files.push_back(entry->d_name);
		closedir(dir);
		buffer = "<html>\n\t<head>\n\t\t<title>Index of " + _uri + "</title>\n\t</head>" + "\n\t<body>\n\t\t<h1>Index of " + _uri + "</h1>\n\t\t<hr>\n\t\t\t<ul>";
		_body.insert(_body.end(), buffer.begin(), buffer.end());
		for (size_t i = 0; i < files.size(); i++)
		{
			buffer = "\n\t\t\t\t<li><a href=\"http://" + _request.get_socket().get_server_socket().get_host() + ":" + _request.get_socket().get_server_socket().get_port() + (!ends_with(_uri, "/") && is_directory((_path).c_str()) ? _uri + "/": _uri) + files[i] + "\">" + files[i] + "</a></li>";
			_body.insert(_body.end(), buffer.begin(), buffer.end());
		}
		buffer = "\n\t\t\t</ul>\n\t\t<hr>\n\t</body>\n</html>\n"; 
		_body.insert(_body.end(), buffer.begin(), buffer.end());
	}

	void response::cgi_process(const std::string &cgi_executable, int in[2], int out[2])
	{
		char **cgi_envp;
		string_vector environment;
		char *const cgi_argv[] = {const_cast<char*>(cgi_executable.c_str()), const_cast<char*>(_path.c_str()), NULL};

		environment.push_back("PATH_INFO=" + _request.get_pathinfo());
		environment.push_back("SERVER_NAME=" + _request[std::string("Host")]);
		environment.push_back("SERVER_PROTOCOL=" HTTP_VERSION);
		environment.push_back("SERVER_PORT=" + _request.get_socket().get_server_socket().get_port());
		environment.push_back("REQUEST_METHOD=" + _request.get_method());
		environment.push_back("SCRIPT_NAME=" + _uri);
		environment.push_back("DOCUMENT_URI=" + _uri);
		// environment.push_back("DOCUMENT_ROOT=" + _location->get_root()); // getcwd() not allowed anymore :(
		environment.push_back("QUERY_STRING=" + _request.get_query());
		environment.push_back("REMOTE_ADDR=" + _request.get_socket().get_host());
		environment.push_back("REMOTE_PORT=" + _request.get_socket().get_port());
		environment.push_back("CONTENT_LENGTH=" + to_string(_request.get_content_length()));
		_request.erase_header("Content-Length");
		if (!_request[std::string("Content-Type")].empty())
		{
			environment.push_back("CONTENT_TYPE=" + _request[std::string("Content-Type")]);
			_request.erase_header("Content-Type");
		}
		environment.push_back("GATEWAY_INTERFACE=CGI/1.1");
		environment.push_back("SERVER_SOFTWARE=webserv/1.0");
		environment.push_back("REQUEST_URI=" + _request.get_uri() + _request.get_pathinfo() + (_request.get_query().empty() ? "" : "?" + _request.get_query()));
		for (string_map::const_iterator it = _request.get_headers().begin(); it != _request.get_headers().end(); it++)
			environment.push_back("HTTP_" + underscore(it->first) + "=" + it->second);
		if (!(cgi_envp = static_cast<char**>(std::calloc(environment.size() + 1, sizeof(char*)))))
			throw std::runtime_error("[CGI] std::calloc() failed.");
		for (size_t i = 0; i < environment.size(); i++)
			cgi_envp[i] = const_cast<char*>(environment[i].c_str());
		if (dup2(in[0], STDIN_FILENO) == -1 || dup2(out[1], STDOUT_FILENO) == -1)
			throw std::runtime_error("[CGI] dup2() failed.");
		close(in[0]), close(in[1]), close(out[0]), close(out[1]);
		execve(cgi_argv[0], cgi_argv, cgi_envp);
		throw std::runtime_error("[CGI] execve() failed: " + std::string(cgi_argv[0][0] == '\0' ? "(null)" : cgi_argv[0]) + " inaccessible.");
	}

	void response::post_method(const std::string &cgi_executable)
	{
		pid_t cgi_pid;
		int term_status, in[2], out[2];
		
		if (cgi_executable.empty())
			throw server_error(internal_server_error);
		if (pipe(in) == -1)
			throw server_error(internal_server_error);
		if (pipe(out) == -1)
		{
			close(in[0]), close(in[1]);
			throw server_error(internal_server_error);
		}
		cgi_pid = fork();
		if (cgi_pid == -1)
		{
			close(in[0]), close(in[1]), close(out[0]), close(out[1]);
			throw server_error(internal_server_error);
		}
		else if (cgi_pid == 0)
			cgi_process(cgi_executable, in, out);
		if (fcntl(in[1], F_SETFL, O_NONBLOCK) == -1 || fcntl(out[0], F_SETFL, O_NONBLOCK) == -1)
		{
			close(in[0]), close(in[1]), close(out[0]), close(out[1]);
			throw server_error(internal_server_error);
		}
		close(in[0]), close(out[1]);
		ssize_t total_bytes_written = 0;
		while (ssize_t bytes_written = write(in[1], &(_request.get_body()[0]) + total_bytes_written, _request.get_content_length() - total_bytes_written))
			if (bytes_written > 0)
				total_bytes_written += bytes_written;
		close(in[1]);
		if (waitpid(cgi_pid, &term_status, 0) == -1 || !WIFEXITED(term_status) || WEXITSTATUS(term_status) != EXIT_SUCCESS)
		{
			close(in[0]), close(out[0]), close(out[1]);
			throw server_error(internal_server_error);
		}
		char buffer[BUFSIZ];
		while (ssize_t bytes_read = read(out[0], buffer, BUFSIZ))
			if (bytes_read > 0)
				_body.insert(_body.end(), buffer, buffer + bytes_read);
		close(out[0]);
	}

	void response::delete_method()
	{
		if (is_regular_file(_path.c_str()))
		{
			if (std::remove(_path.c_str()) == 0)
				_status = no_content;
			else throw server_error(internal_server_error);
		}
		else if (errno == ENOENT)
			throw server_error(not_found);
		else throw server_error(forbidden);
	}

	void response::print_response() const
	{
		std::cout << GREEN BOLDED("------- start of sent response -------") << LGREEN << std::endl;
		write(STDOUT_FILENO, &_message.front(), _message.size());
		std::cout << GREEN BOLDED("------- end of sent response ---------") RESET << std::endl;
	}

	bool response::is_regular_file(const char *filename) const
	{
		DIR *directory = opendir(filename);

		if (directory)
		{
			closedir(directory);
			return (false);
		}
		return (errno == ENOTDIR ? true : false);
	}

	bool response::is_directory(const char *filename) const
	{
		DIR *directory = opendir(filename);

		if (directory)
		{
			closedir(directory);
			return (true);
		}
		return (false);
	}

	std::string response::append_trailing_slash(const std::string &path)
	{
		if (!ends_with(path, "/") && is_directory(path.c_str()))
			return (path + "/");
		return (path);
	}
}
