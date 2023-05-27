#include "response.hpp"

namespace ft
{
	response::response(const response &other) : _status(other._status), _body(other._body), _headers(other._headers), _message(other._message), _uri(other._uri), _request(other._request), _location(other._location), _cursor(other._cursor), _path(other._path) {}

	response::response(const request &request, http_code status) : _status(status), _body(), _headers(), _message(), _uri(request.get_uri()), _request(request), _location(), _cursor(), _path()
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

	void response::construct_error_page(int error_code)
	{
		std::stringstream code;

		code << error_code;
		_body = "<html>\n\t<head>\n\t\t<title>Error " + code.str() + "</title>\n\t</head>"
				+ "\n\t<body>\n\t\t<h1>Error " + code.str() + " " + reason_phrase(_status) + "</h1>\n\t</body>\n</html>\n";
	}

	void response::read_error_page(int error_code, bool loc) //check how the path is constructed
	{
		std::string error_page;
		
		if (!loc)
			error_page = _request.get_server().get_root() + _request.get_server().get_error_page(error_code);	//root?
		else
		{
			std::cout << "I am here\n";
			// std::cout << _location->get_route() + _location->get_error_page(error_code) << std::endl;
			error_page = _location->get_root() + _location->get_route() + _location->get_error_page(error_code);
			std::cout << CYAN "get_error_page: " << _location->get_error_page(error_code) << RESET << std::endl;
			std::cout << RED "error_code: " << error_code << RESET << std::endl;
			std::cout << RED "location route: " << _location->get_route() << RESET << std::endl;
			std::cout << "error page to be requested: " << error_page << std::endl;
		}
		// std::cout << "Server root: " << _request.get_server().get_root() << std::endl;
		// std::cout << "Error page: " << error_page << std::endl;
		if (error_page.empty() || !read_requested_file(error_page))
		{
			// std::cout << "Could not open file\n";
			construct_error_page(error_code);
		}
	}

	void response::get_method()
	{
		find_requested_file();
	}

	bool response::read_requested_file(const std::string &filename)
	{
		std::ifstream file;

		std::cout << "Read requested file: " << filename << std::endl;
		if (!is_regular_file(filename.c_str()))
			return (false);
		file.open(filename);
		if (file)
		{
			std::ostringstream ss;

			_path = filename;
			ss << file.rdbuf();
			_body = ss.str();
			if (_body.length() != 0)
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
						break;
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
		_headers["Content-Length"] = to_string(_body.length());
		if (!_status)
			_status = ok;
		std::stringstream ss;

		_message = HTTP_VERSION " ";
		ss << _status;
		_message += ss.str() + " " + reason_phrase(_status) + CRLF;
		for (string_map::const_iterator it = _headers.begin(); it != _headers.end(); it++)
			_message += it->first + ": " + it->second + CRLF;
		_message += CRLF + _body;
		// std::cout << "response of size " << _message.size() << " is:" << std::endl;
		// std::cout << YELLOW << _message.substr(0, 300) + "..." << RESET << std::endl;
	}

	std::string response::get_chunk()
	{
		std::string chunk = _message.substr(_cursor, BUFSIZ);
		_cursor = std::min(_cursor + BUFSIZ, _message.size());
		return (chunk);
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
		std::vector<std::string> files;

		if (dir == nullptr)
			throw server::server_error(not_found); //does autoindex have it's own error code??

		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
			if (entry->d_name[0] != '.')
				files.push_back(entry->d_name);

		closedir(dir);
		_body = "<html>\n\t<head>\n\t\t<title>Index of " + _uri + "</title>\n\t</head>"
					+ "\n\t<body>\n\t\t<h1>Index of " + _uri + "</h1>\n\t\t<hr>\n\t\t\t<ul>";

		for (size_t i = 0; i < files.size(); i++)
		{
			std::cout << MAGENTA "path is " << _path << std::endl;
			std::cout << "uri is " << _uri << RESET << std::endl;
			_body += "\n\t\t\t\t<li><a href=\"http://" + _request.get_socket().get_server_socket().get_host() + ":" + _request.get_socket().get_server_socket().get_port() + (!ends_with(_uri, "/") && is_directory((_path).c_str()) ? _uri + "/": _uri) + files[i] + "\">" + files[i] + "</a></li>";
			// std::cout << "Href: http://" << _request.get_socket().get_server_socket().get_host() << ":" << _request.get_socket().get_server_socket().get_port() << (!ends_with(_uri, "/") && is_directory((_path).c_str()) ? _uri + "/": _uri) << files[i] << std::endl;
		}

		_body += "\n\t\t\t</ul>\n\t\t<hr>\n\t</body>\n</html>\n"; 
	}

	void response::post_method(const std::string &cgi_executable)
	{
		if (cgi_executable.empty())
		{
			_status = no_content;
			return ;
		}
		char **serv_env = webserver.get_environ(), **cgi_env;
		const char *cgi_path = cgi_executable.c_str();
		char *const cgi_args[] = {const_cast<char*>(cgi_executable.c_str()), const_cast<char*>(_path.c_str()), NULL};
		{
			string_vector environment;
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
			environment.push_back("REQUEST_URI=" + _request.get_uri() + (_request.get_query().empty() ? "" : "?" + _request.get_query()));
			if (!(cgi_env = static_cast<char**>(std::calloc(environment.size() + 1, sizeof(char*)))))
				throw server::server_error(internal_server_error);
			for (size_t i = 0; i < environment.size(); i++)
				if (!(cgi_env[i] = strdup(environment[i].c_str())))
				{
					for (size_t j = 0; j < i; j++)
						std::free(cgi_env[j]);
					std::free(cgi_env);
					break ;
				}
		}
		try
		{
			std::cout << MAGENTA "running execve() on " << cgi_args[0] << ", " << cgi_args[1] << RESET << std::endl;
			execute_cgi(cgi_path, cgi_args, cgi_env);
		}
		catch (...)
		{
			for (size_t i = 0; cgi_env[i] != NULL; i++)
				std::free(cgi_env[i]);
			std::free(cgi_env);
			throw ;
		}
	}

	void response::execute_cgi(const char *cgi_path, char *const cgi_args[], char **cgi_env)
	{
		pid_t cgi_pid;
		int term_status;
		ssize_t bytes_written;
		ssize_t bytes_read = 1; // to activate the loop
		int in_pipe[2], out_pipe[2];

		if (pipe(in_pipe) == -1)
			throw server::server_error(internal_server_error);
		pipe_failsafe(pipe(out_pipe), in_pipe);
		cgi_pid = fork();
		if (cgi_pid == -1)
			pipe_failsafe(cgi_pid, in_pipe, out_pipe);
		else if (cgi_pid == 0) // CGI child process code
		{
			close(in_pipe[1]);
			close(out_pipe[0]);
			if (dup2(in_pipe[0], STDIN_FILENO) == -1 || dup2(out_pipe[1], STDOUT_FILENO) == -1)
				throw server::server_error(internal_server_error);
			close(in_pipe[0]);
			close(out_pipe[1]);
			execve(cgi_path, cgi_args, cgi_env);
			throw server::server_error(internal_server_error);
		}
		close(in_pipe[0]);
		close(out_pipe[1]);
		bytes_written = write(in_pipe[1], &_request.get_body().front(), _request.get_content_length());
		if (bytes_written != _request.get_content_length())
		{
			kill(cgi_pid, SIGTERM);
			close(in_pipe[1]);
			close(out_pipe[0]);
			throw server::server_error(internal_server_error);
		}
		close(in_pipe[1]);
		while (bytes_read > 0)
		{
			char buffer[BUFSIZ] = {0};
			bytes_read = read(out_pipe[0], buffer, BUFSIZ - 1);
			if (bytes_read <= 0) break ;
			std::cerr << MAGENTA "BUFFER ATTACHED TO BODY IS |" << buffer << "|" RESET << std::endl;
			_body += buffer;
		}
		if (bytes_read == -1)
		{
			kill(cgi_pid, SIGTERM);
			close(out_pipe[0]);
			_body.clear();
			throw server::server_error(internal_server_error);
		}
		close(out_pipe[0]);
		if (waitpid(cgi_pid, &term_status, 0) == -1 || !WIFEXITED(term_status) || WEXITSTATUS(term_status) != 0)
			throw server::server_error(internal_server_error);
	}

	void response::pipe_failsafe(int status, int in_pipe[2], int out_pipe[2])
	{
		if (status == -1)
		{
			if (in_pipe)
			{
				close(in_pipe[0]);
				close(in_pipe[1]);
			}
			if (out_pipe)
			{
				close(out_pipe[0]);
				close(out_pipe[1]);
			}
			throw server::server_error(internal_server_error);
		}
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
