#include "response.hpp"

namespace ft
{
	response::response(const response &other) : _request(other._request), _status(other._status), _headers(other._headers), _uri(other._uri), _path(other._path), _body(other._body), _message(other._message), _cursor(other._cursor), _location(other._location) {}

	response::response(const request &request, http_code status) : _request(request), _status(status), _headers(), _uri(request.get_uri()), _path(), _body(), _message(), _cursor(), _location()
	{
		generate_response();
	}

	response::~response() {}

	response &response::operator=(const response &other)
	{
		_status = other._status;
		_body = other._body;
		_headers = other._headers;
		_message = other._message;
		_location = other._location;
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
				throw server::server_error(_status);
			find_rewritten_location();
			_path = append_trailing_slash(_location->get_root() + _uri);
			const std::string cgi = _location->get_cgi_executable(get_file_extension(_uri));
			if (_request.get_method() == "GET")
			{
				std::cout << "GET METHOD: \n";
				if (!cgi.empty())
				{
					std::cout << "Post case\n";
					post_method(cgi);
				}
				else get_method();
			}
			else if (_request.get_method() == "POST")
				post_method(cgi);
			else if (_request.get_method() == "DELETE")
				delete_method();
			else throw server::server_error(not_implemented);
		}
		catch (const server::server_error &e)
		{
			// std::cout << "Path: " << _path << "\n";
			// throw std::exception();
			// std::cout << "In catch block\n";
			_status = e;

			if (e == 400 || e == 505 || _location == NULL) //bad request or http version not supported
				read_error_page(e, false);
			else if (!read_requested_file(_location->get_error_page(e)))
				if (!read_requested_file(_location->get_error_page(404)))
					read_error_page(e);
			//add_headers();
		}
		construct_response();
	}

	void response::construct_error_page(http_code error)
	{
		std::string content = "<html>\n\t<head>\n\t\t<title>Error " + to_string(error) + "</title>\n\t</head>"
				+ "\n\t<body>\n\t\t<h1>Error " + to_string(error) + " " + reason_phrase(_status) + "</h1>\n\t</body>\n</html>\n";
		_body.insert(_body.end(), content.begin(), content.end());
	}

	void response::read_error_page(http_code error, bool loc) //check how the path is constructed
	{
		std::string error_page;
		
		if (!loc)
			error_page = _request.get_server().get_root() + _request.get_server().get_error_page(error);	//root?
		else
		{
			std::cout << "I am here\n";
			// std::cout << _location->get_route() + _location->get_error_page(error_code) << std::endl;
			error_page = _location->get_root() + _location->get_route() + _location->get_error_page(error);
			std::cout << CYAN "get_error_page: " << _location->get_error_page(error) << RESET << std::endl;
			std::cout << RED "error_code: " << error << RESET << std::endl;
			std::cout << RED "location route: " << _location->get_route() << RESET << std::endl;
			std::cout << "error page to be requested: " << error_page << std::endl;
		}
		// std::cout << "Server root: " << _request.get_server().get_root() << std::endl;
		// std::cout << "Error page: " << error_page << std::endl;
		if (error_page.empty() || !read_requested_file(error_page))
		{
			// std::cout << "Could not open file\n";
			construct_error_page(error);
		}
	}

	void response::get_method()
	{
		find_requested_file();
	}

	bool response::read_requested_file(const std::string &filename)
	{
		std::ifstream file;

		// std::cout << "Read requested file: " << filename << std::endl;
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
			std::copy(std::istream_iterator<char>(file), std::istream_iterator<char>(), std::back_inserter(_body));
			std::cout << "body size after read: " << _body.size() << " while file_len is " << file_len << std::endl;
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
			if (!_location->get_indices().empty())
			{
				ft::string_vector::const_iterator it = _location->get_indices().begin();
				for (; it != _location->get_indices().end(); it++)
				{
					std::cout << "Index file: " << *it << std::endl;
					if (read_requested_file(_path + *it))
						break ;
				}
				if (it == _location->get_indices().end())
				{
					if (_location->get_autoindex())
						generate_autoindex(_path);
					else throw server::server_error(forbidden);
				}
			}
			// else
			// {
			// 	std::cout << "MTELLLL EM AYSSS ELSE_i meeejjjj\n";
			// 	if (_location->get_autoindex())
			// 		generate_autoindex(_path);
			// 	else
			// 		throw server::server_error(not_found);
			// }
		}
		else
			if (!read_requested_file(_path))
				throw server::server_error(not_found);
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
			throw server::server_error(not_found);
		}
	}

	void response::construct_response()
	{
		_headers["Content-Length"] = to_string(_body.size());
		if (!_status)
			_status = ok;
		std::string buffer = HTTP_VERSION " " + to_string(_status) + " " + reason_phrase(_status) + CRLF;
		_message.insert(_message.end(), buffer.begin(), buffer.end());
		for (string_map::const_iterator it = _headers.begin(); it != _headers.end(); it++)
		{
			buffer = it->first + ": " + it->second + CRLF;
			_message.insert(_message.end(), buffer.begin(), buffer.end());
		}
		buffer = CRLF;
		_message.insert(_message.end(), buffer.begin(), buffer.end());
		_message.insert(_message.end(), _body.begin(), _body.end());
		_body.clear();
		// std::cout << "response of size " << _message.size() << " is:" << std::endl;
		// for (size_t i = 0; i < _message.size(); i++) std::cout << YELLOW << _message[i] << RESET;
		std::cout << std::endl;
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
		std::cout << MAGENTA "tryna replace " << what << " with " << with_what << RESET << std::endl;
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
		std::cout << LGREEN "uri before rewrite: " << _uri << RESET << std::endl;
		for (ft::string_mmap::const_iterator it = _request.get_server().get_rewrites().begin(); it != _request.get_server().get_rewrites().end(); it++)
			rewrite(it->first, it->second);
		std::cout << LGREEN "uri after rewrite: " << _uri << RESET << std::endl;
		int i = 0;
		find_location(_request.get_server());
		while (i != 10)
		{
			if (!_location->get_rewrites().empty())
			{
				for (ft::string_mmap::const_iterator it = _location->get_rewrites().begin(); it != _location->get_rewrites().end(); it++)
					rewrite(it->first, it->second);
				find_location(_request.get_server());
				i++;
			}
			else
				break;
		}
		if (!_location->is_allowed_method(_request.get_method()))
		{
			// std::cout << "Method: " << _request.get_method() << std::endl;
			// std::cout << "Shoud be here\n";
			throw server::server_error(method_not_allowed);
		}
		if (_location->get_client_max_body_size() < _request.get_body().size())
			throw server::server_error(content_too_large);
	}

	void response::generate_autoindex(const std::string &path) 
	{
		DIR *dir = opendir(path.c_str());
		string_vector files;
		std::string buffer;

		if (dir == NULL)
			throw server::server_error(not_found);
		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
			if (entry->d_name[0] != '.')
				files.push_back(entry->d_name);
		closedir(dir);
		buffer = "<html>\n\t<head>\n\t\t<title>Index of " + _uri + "</title>\n\t</head>" + "\n\t<body>\n\t\t<h1>Index of " + _uri + "</h1>\n\t\t<hr>\n\t\t\t<ul>";
		_body.insert(_body.end(), buffer.begin(), buffer.end());
		for (size_t i = 0; i < files.size(); i++)
		{
			std::cout << MAGENTA "path is " << _path << std::endl;
			std::cout << "uri is " << _uri << RESET << std::endl;
			buffer = "\n\t\t\t\t<li><a href=\"http://" + _request.get_socket().get_server_socket().get_host() + ":" + _request.get_socket().get_server_socket().get_port() + (!ends_with(_uri, "/") && is_directory((_path).c_str()) ? _uri + "/": _uri) + files[i] + "\">" + files[i] + "</a></li>";
			_body.insert(_body.end(), buffer.begin(), buffer.end());
		}
		buffer = "\n\t\t\t</ul>\n\t\t<hr>\n\t</body>\n</html>\n"; 
		_body.insert(_body.end(), buffer.begin(), buffer.end());
	}

	void response::cgi_process(const std::string &cgi_executable, const std::string &tmp_in, const std::string &tmp_out)
	{
		int	fd_in, fd_out;
		char *const cgi_argv[] = {const_cast<char*>(cgi_executable.c_str()), const_cast<char*>(_path.c_str()), NULL};
		char **serv_env = webserver.get_environ(), **cgi_env;
		{
			char *pwd = getenv("PWD");
			string_vector environment;
			if (pwd) environment.push_back("PATH_TRANSLATED=" + std::string(pwd) + "/" + _uri);
			for (size_t i = 0; serv_env[i] != NULL; i++)
				environment.push_back(serv_env[i]);
			environment.push_back("SERVER_NAME=" + _request[std::string("Host")]);
			environment.push_back("SERVER_PROTOCOL=" HTTP_VERSION);
			environment.push_back("SERVER_PORT=" + _request.get_socket().get_server_socket().get_port());
			environment.push_back("REQUEST_METHOD=" + _request.get_method());
			environment.push_back("SCRIPT_NAME=" + _uri);
			environment.push_back("DOCUMENT_ROOT=" + _location->get_root());
			environment.push_back("QUERY_STRING=" + _request.get_query());
			environment.push_back("REMOTE_ADDR=" + _request.get_socket().get_host());
			environment.push_back("CONTENT_LENGTH=" + to_string(_request.get_content_length()));
			environment.push_back("CONTENT_TYPE=" + _request[std::string("Content-Type")]);
			environment.push_back("GATEWAY_INTERFACE=CGI/1.1");
			environment.push_back("SERVER_SOFTWARE=webserv/1.0");
			environment.push_back("REQUEST_URI=" + _request.get_uri() + (_request.get_query().empty() ? "" : "?" + _request.get_query()));
			if (!(cgi_env = static_cast<char**>(std::calloc(environment.size() + 1, sizeof(char*)))))
			{
				webserver.error("Bad memory allocation for CGI environ.");
				throw server::server_error(internal_server_error);
			}
			for (size_t i = 0; i < environment.size(); i++)
				cgi_env[i] = const_cast<char*>(environment[i].c_str());
		}
		std::remove(tmp_in.c_str());
		std::remove(tmp_out.c_str());
		{
			std::ofstream file(tmp_in);
			if (file.is_open())
			{
				file.write(&_request.get_body().front(), _request.get_content_length());
				if (!file.good())
					throw server::server_error(internal_server_error);
			}
			else throw server::server_error(internal_server_error);
		}
		if ((fd_in = open(tmp_in.c_str(), O_RDWR | O_CREAT | O_TRUNC, 00755)) == -1 ||
			(fd_out = open(tmp_out.c_str(), O_RDWR | O_CREAT | O_TRUNC, 00755)) == -1)
		{
			webserver.error("Bad temporary file open()ing.");
			throw server::server_error(internal_server_error);
		}
		if (dup2(fd_in, STDIN_FILENO) == -1 || dup2(fd_out, STDOUT_FILENO) == -1)
		{
			webserver.error("Bad dup2()ing.");
			throw server::server_error(internal_server_error);
		}
		execve(cgi_argv[0], cgi_argv, cgi_env);
		webserver.error("Bad CGI execve().");
		throw server::server_error(internal_server_error);
	}

	void response::post_method(const std::string &cgi_executable)
	{
		pid_t cgi_pid, wait_ret;
		int term_status;
		const std::string tmp_in = "/tmp/webserv_in" + to_string(_request);
		const std::string tmp_out = "/tmp/webserv_out" + to_string(_request);

		cgi_pid = fork();
		if (cgi_pid == -1)
			throw server::server_error(internal_server_error);
		else if (cgi_pid == 0) // CGI child process code
			cgi_process(cgi_executable, tmp_in, tmp_out);
		// std::cout << "Request content length: " << _request.get_content_length() << std::endl;
		wait_ret = waitpid(cgi_pid, &term_status, 0);
		std::remove(tmp_in.c_str());
		if (wait_ret == -1 || !WIFEXITED(term_status))
		{
			std::remove(tmp_out.c_str());
			throw server::server_error(internal_server_error);
		}
		// ... attach tmp_out to response body
		// std::cout << MAGENTA "good wait" RESET << std::endl;
	}

	void response::delete_method()
	{
		if (is_regular_file(_path.c_str()))
		{
			if (std::remove(_path.c_str()) == 0)
				_status = no_content;
			else
				throw server::server_error(internal_server_error);
		}
		else
			throw server::server_error(forbidden);
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
