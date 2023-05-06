#include "response.hpp"		//change later

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
		try
		{
			if (http::is_error_code(_status))
				throw server::server_error(_status, "Request error.");
			find_rewritten_location();
			_path = _location->get_root() + _uri;
			if (this->_request.get_method() == "GET")
				get_method();
			else if (this->_request.get_method() == "POST")
				post_method();
		}
		catch (const server::server_error &e)
		{
			this->_status = e;

			if (e == 400 || e == 505) //bad request or http version not supported
				read_error_page(e, false);
			if (!read_requested_file(_location->get_error_page(e)))
				if (!read_requested_file(_location->get_error_page(404)))
					read_error_page(e);
			//add_headers();
			construct_response();
		}
	}

	void response::construct_error_page(int error_code)
	{
		std::stringstream code;

		code << error_code;
		_body = "<html>\n\t<head>\n\t\t<title>Error " + code.str() + "</title>\n\t</head>"
				+ "\n\t<body>\n\t\t<h1>Error " + code.str() + " " + reason_phrase(this->_status) +" + </h1>\n\t</body>\n</html>\n";
	}

	void response::read_error_page(int error_code, bool loc) //check how the path is constructed
	{
		std::string error_page;
		if (!loc)
			error_page = _request.get_server().get_root() + _request.get_server().get_error_page(error_code);	//root?
		else
			error_page = _location->get_route() + _location->get_error_page(error_code);
		if (!error_page.length() || !read_requested_file(error_page))
			construct_error_page(error_code);
	}

	void response::get_method()
	{
		find_requested_file();
		//add_headers();
		construct_response();
	}

	bool response::read_requested_file(const std::string &path)
	{
		std::ifstream file;

		file.open(path);
		if (file.is_open())
		{
			_path = path;
			file >> _body;
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
			if (!_location->get_indexes().empty())
			{
				for (ft::string_vector::const_iterator it = _location->get_indexes().begin(); it != _location->get_indexes().end(); it++)
					if (read_requested_file(_path + *it))
						break;
			}
			else
			{
				if (_location->get_autoindex())
					generate_autoindex(_path);
				else
					throw server::server_error(not_found, "File not found.");
			}
		}
		else
			if (!read_requested_file(_path))
				throw server::server_error(not_found, "File not found.");
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
			throw server::server_error(not_found, "File not found.");
	}

	void response::construct_response()
	{
		_headers["Content-Length"] = _body.length();
		if (!_status)
			_status = ok;
		std::stringstream ss;

		this->_message = HTTP_VERSION " ";
		ss << this->_status;
		this->_message += ss.str() + " " + reason_phrase(this->_status) + CRLF;
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

	bool response::rewrite(const std::string &what, const std::string &with_what)
	{
		size_t pos = _uri.find(what);
		if (pos)
		{
			_uri.replace(pos, pos + what.length(), with_what);
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
		if (!_location->is_allowed_method(_request.get_method()))
			throw server::server_error(method_not_allowed, "Method not allowed.");
		if (this->_location->get_client_max_body_size() < this->_request.get_body().size())
			throw server::server_error(content_too_large, "Exceeded \"client_max_body_size\".");
	}

	void response::generate_autoindex(const std::string &path) 
	{
		DIR *dir = opendir(path.c_str());
		std::vector<std::string> files;

		if (dir == nullptr)
			throw server::server_error(not_found, "File not found."); //does autoindex have it's own error code??

		struct dirent *entry;
		while ((entry = readdir(dir)) != nullptr)
			if (entry->d_name[0] != '.')
				files.push_back(entry->d_name);

		closedir(dir);
		_body = "<html>\n\t<head>\n\t\t<title>Index of " + path + "</title>\n\t</head>"
					+ "\n\t<body>\n\t\t<h1>Index of " + path + "</h1>\n\t\t<hr>\n\t\t\t<ul>";

		for (size_t i = 0; i < files.size(); i++)
			_body += "\n\t\t\t\t<li><a href=\"http://" + _request.get_socket().get_host() + ":" + _request.get_socket().get_port() + path + files[i] + "\">" + files[i] + "</a></li>";

		_body += "\n\t\t\t</ul>\n\t\t<hr>\n\t</body>\n</html>\n"; 
	}

	void response::post_method()
	{
		const std::string cgi = this->_location->get_cgi_param("SCRIPT_FILENAME");
		if (cgi.empty())
		{
			this->_status = no_content;
			return ;
		}
		char **serv_env = webserver.get_environ();
		char *cgi_path;
		char **cgi_args;
		char **cgi_env;

		malloc_failsafe(cgi_path = strdup(cgi.c_str()));
		malloc_failsafe(cgi_args = static_cast<char**>(std::calloc(3, sizeof(char*))), cgi_path);
		malloc_failsafe(cgi_args[0] = strdup(cgi.c_str()), cgi_path, cgi_args);
		malloc_failsafe(cgi_args[1] = strdup(this->_path.c_str()), cgi_path, cgi_args[0], cgi_args);
		{
			string_vector environment;

			for (size_t i = 0; serv_env[i] != NULL; i++)
				environment.push_back(serv_env[i]);
			environment.push_back("SERVER_NAME=" + this->_request[std::string("Host")]);
			environment.push_back("SERVER_PROTOCOL=" HTTP_VERSION);
			environment.push_back("SERVER_PORT=" + this->_request.get_socket().get_server_socket().get_port());
			environment.push_back("REQUEST_METHOD=" + this->_request.get_method());
			environment.push_back("SCRIPT_NAME=" + this->_location->get_cgi_param("SCRIPT_NAME"));
			environment.push_back("DOCUMENT_ROOT=" + this->_location->get_root());
			if (!this->_request.get_query().empty())
				environment.push_back("QUERY_STRING=" + this->_request.get_query());
			environment.push_back("REMOTE_ADDR=" + this->_request.get_socket().get_host());
			environment.push_back("CONTENT_LENGTH=" + to_string(this->_request.get_content_length()));
			environment.push_back("PATH_INFO=" + this->_path);
			environment.push_back("REQUEST_URI=" + this->_request.get_uri() + (this->_request.get_query().empty() ? "" : "?" + this->_request.get_query()));
			malloc_failsafe(cgi_env = static_cast<char**>(std::calloc(environment.size() + 1, sizeof(char*))), cgi_path, cgi_args[0], cgi_args[1], cgi_args);
			for (size_t i = 0; i < environment.size(); i++)
			{
				cgi_env[i] = strdup(environment[i].c_str());
				if (!cgi_env[i])
				{
					for (size_t j = 0; j < i; j++)
						std::free(cgi_env[j]);
					malloc_failsafe(NULL, cgi_path, cgi_args[0], cgi_args[1], cgi_args, cgi_env);
				}
			}
		}
		try
		{
			execute_cgi(cgi_path, cgi_args, cgi_env);
		}
		catch (...)
		{
			free_cgi_args(cgi_path, cgi_args, cgi_env);
			throw ;
		}
		free_cgi_args(cgi_path, cgi_args, cgi_env);
	}

	void response::execute_cgi(char *cgi_path, char **cgi_args, char **cgi_env)
	{
		pid_t cgi_pid;
		int term_status;
		ssize_t bytes_written;
		ssize_t bytes_read = 1; // to activate the loop
		int in_pipe[2], out_pipe[2];

		if (pipe(in_pipe) == -1)
			throw server::server_error(internal_server_error, "Exceptional error while attempting to run CGI.");
		pipe_failsafe(pipe(out_pipe), in_pipe);
		cgi_pid = fork();
		if (cgi_pid == -1)
			pipe_failsafe(cgi_pid, in_pipe, out_pipe);
		else if (cgi_pid == 0) // CGI child process code
		{
			close(in_pipe[1]);
			close(out_pipe[0]);
			if (dup2(in_pipe[0], STDIN_FILENO) == -1 || dup2(out_pipe[1], STDOUT_FILENO) == -1)
				throw server::server_error(internal_server_error, "Exceptional error while attempting to run CGI.");
			close(in_pipe[0]);
			close(out_pipe[1]);
			execve(cgi_path, cgi_args, cgi_env);
			throw server::server_error(internal_server_error, "CGI file not found.");
		}
		close(in_pipe[0]);
		close(out_pipe[1]);
		bytes_written = write(in_pipe[1], this->_request.get_body().c_str(), this->_request.get_content_length());
		if (bytes_written != this->_request.get_content_length())
		{
			kill(cgi_pid, SIGTERM);
			close(in_pipe[1]);
			close(out_pipe[0]);
			throw server::server_error(internal_server_error, "Exceptional error while attempting to run CGI.");
		}
		close(in_pipe[1]);
		while (bytes_read > 0)
		{
			char buffer[BUFSIZ] = {0};
			bytes_read = read(out_pipe[0], buffer, BUFSIZ - 1);
			if (bytes_read <= 0) break ;
			this->_body += buffer;
		}
		if (bytes_read == -1)
		{
			kill(cgi_pid, SIGTERM);
			close(out_pipe[0]);
			this->_body.clear();
			throw server::server_error(internal_server_error, "Exceptional error while attempting to run CGI.");
		}
		close(out_pipe[0]);
		if (waitpid(cgi_pid, &term_status, 0) == -1 || !WIFEXITED(term_status) || WEXITSTATUS(term_status) != 0)
			throw server::server_error(internal_server_error, "Exceptional error while attempting to run CGI.");
	}

	void pipe_failsafe(int status, int in_pipe[2], int out_pipe[2])
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
			throw server::server_error(internal_server_error, "Exceptional error while attempting to run CGI.");
		}
	}

	void response::malloc_failsafe(void *memory, void *mem1, void *mem2, void *mem3, void *mem4, void *mem5)
	{
		if (!memory)
		{
			std::free(mem1), std::free(mem2), std::free(mem3), std::free(mem4), std::free(mem5);
			throw server::server_error(internal_server_error, "Not enough memory to run CGI.");
		}
	}

	void response::free_cgi_args(char *cgi_path, char **cgi_args, char **cgi_env)
	{
		for (size_t i = 0; cgi_env[i] != NULL; i++)
			std::free(cgi_env[i]);
		std::free(cgi_env), std::free(cgi_args[0]), std::free(cgi_args[1]), std::free(cgi_args), std::free(cgi_path);
	}

	void response::delete_method()
	{
		if (is_regular_file(this->_path.c_str()))
		{
			if (std::remove(this->_path.c_str()) == 0)
				this->_status = no_content;
			else
				throw server::server_error(internal_server_error, "Failed to delete a resource.");
		}
		else
			throw server::server_error(forbidden, "Deleting non-regular files is not allowed.");
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
}
