#include "parser.hpp"

namespace ft
{
	parser::parser(): _directives(), _contexts(), _config(), _chunks() {}

	parser::parser(const std::string &filename): _directives(), _contexts(), _config(), _chunks()
	{
		_config.open(filename.c_str());
		if (!_config.is_open())
			throw std::runtime_error("File \"" + filename + "\" does not exist.");
		_contexts.insert(context("http", cont_functor(&parser::process_http, true)));
		_contexts.insert(context("server", cont_functor(&parser::process_server, false)));
		_contexts.insert(context("location", cont_functor(&parser::process_location, false)));
		_directives.insert(directive("root", dir_functor(&parser::read_root, false)));
		_directives.insert(directive("autoindex", dir_functor(&parser::read_autoindex, false)));
		_directives.insert(directive("error_page", dir_functor(&parser::read_error_page, false)));
		_directives.insert(directive("client_max_body_size", dir_functor(&parser::read_client_max_body_size, false)));
		_directives.insert(directive("index", dir_functor(&parser::read_index, false)));
		_directives.insert(directive("listen", dir_functor(&parser::read_listen, false)));
		_directives.insert(directive("server_name", dir_functor(&parser::read_server_name, false)));
		_directives.insert(directive("rewrite", dir_functor(&parser::read_rewrite, false)));
		_directives.insert(directive("cgi_executable", dir_functor(&parser::read_cgi_executable, false)));
		_directives.insert(directive("cgi_extension", dir_functor(&parser::read_cgi_extension, false)));
		_directives.insert(directive("limit_except", dir_functor(&parser::read_limit_except, false)));
		std::cout << CYAN "SET MAP" RESET << std::endl;
		parse_chunks();
	}

	parser::~parser() {}

	parser::parser(const parser &other): _directives(other._directives), _contexts(other._contexts), _config(), _chunks(other._chunks) {}

	parser &parser::operator=(const parser&) { return (*this); }

	base_dir *parser::parse(base_dir *parent)
	{
		context_map::iterator cont_it = _contexts.begin();
		for (; cont_it != _contexts.end(); cont_it++)
			if (cont_it->second.second && is_context(cont_it->first))
			{
				parent = (this->*(cont_it->second.first))(parent);
				break ;
			}
		directive_map::iterator dir_it = _directives.begin();
		for (; dir_it != _directives.end(); dir_it++) 
			if (dir_it->second.second && is_directive(dir_it->first))
			{
				erase_token_front(";", (this->*(dir_it->second.first))(parent));
				break ;
			}
		if (cont_it == _contexts.end() && dir_it == _directives.end() && !_chunks.empty())
			throw parsing_error("Unknown directive or ill-formed context encountered.");
		return (parent);
	}

	bool parser::is_context(std::string context)
	{
		string_vector location_args;

		if (front().substr(0, context.length()) == context)
		{
			bool brace_erased = false;
			if (erase_chunk_front(context) && context == "location")
			{
				if (front() == "=")
					location_args.push_back(pop_front());
				brace_erased = erase_chunk_middle("{");
				location_args.push_back(pop_front());
			}
			else if (context == "location")
				throw parsing_error("Invalid `location` syntax.");
			erase_token_front("{", brace_erased);
			for (size_t i = 0; i < location_args.size(); i++)
				_chunks.push_front(location_args[i]);
			return (true);
		}
		return (false);
	}

	bool parser::is_directive(std::string directive)
	{
		if (front().substr(0, directive.length()) != directive)
			return (false);
		if (!erase_chunk_front(directive))
			throw parsing_error("Unknown directive provided.");
		return (true);
	}

	void parser::parse_chunks()
	{
		std::string line;

		while (std::getline(_config, line))
		{
			std::string chunk;
			std::stringstream buffer(line);
			while (buffer >> chunk)
			{
				const size_t hash = chunk.find('#');
				if (hash != std::string::npos)
				{
					_chunks.push_front(chunk);
					erase_chunk_middle("#", true);
					if (front().empty())
						pop_front();
					else
						_chunks.push_back(pop_front());
					if (hash + 1 != chunk.length())
						pop_front();
					buffer.str(""); // clear buffer
				}
				else
					_chunks.push_back(chunk);
			}
		}
	}

	base_dir *parser::process_http(base_dir*)
	{	
		http *protocol = new http();
		
		std::cout << CYAN "http INDICES OF SIZE " << protocol->get_indices().size() << RESET << std::endl;

		try
		{
			_contexts["http"].second = false;
			_contexts["server"].second = true;
			load_base_dir();
			while (front().at(0) != '}')
				parse(protocol);
			erase_chunk_front("}");
			unload_base_dir();
			_contexts["server"].second = false;
			parse(protocol);
			open_sockets(protocol);
		}
		catch (...)
		{
			delete protocol;
			throw ;
		}
		return (protocol);
	}

	base_dir *parser::process_server(base_dir *protocol)
	{
		std::cout << YELLOW "before construction" RESET << std::endl;
		server serv(*protocol);
		std::cout << YELLOW "after construction" RESET << std::endl;
		std::cout << CYAN "http at " MAGENTA << protocol << RESET << std::endl;
		std::cout << CYAN "server at " MAGENTA << &serv << RESET << std::endl;
		std::cout << CYAN "http INDICES OF SIZE " << protocol->get_indices().size() << RESET << std::endl;
		std::cout << CYAN "http INDICES UNDER " << &protocol->get_indices() << RESET << std::endl;
		std::cout << CYAN "server INDICES OF SIZE " << serv.get_indices().size() << RESET << std::endl;
		std::cout << CYAN "server INDICES UNDER " << &serv.get_indices() << RESET << std::endl;
		_contexts["server"].second = false;
		_contexts["location"].second = true;
		_directives["listen"].second = true;
		_directives["server_name"].second = true;
		_directives["rewrite"].second = true;

		while (front().at(0) != '}')
		{
			std::cout << CYAN "about to parse " << front() << RESET << std::endl;
			
			parse(&serv);
		}
		erase_chunk_front("}");

		if (serv.get_names().empty())
			serv.add_name("");
		if (std::find(serv.get_locations().begin(), serv.get_locations().end(), location(serv, "/")) == serv.get_locations().end())
			serv.add_location(location(serv, "/"));

		static_cast<http*>(protocol)->add_server(serv);
		map_sockets(&static_cast<http*>(protocol)->get_servers().back());

		_directives["rewrite"].second = false;
		_directives["server_name"].second = false;
		_directives["listen"].second = false;
		_contexts["location"].second = false;
		_contexts["server"].second = true;
		return (protocol);
	}

	base_dir *parser::process_location(base_dir *parent)
	{
		location loc(*parent);
		_directives["limit_except"].second = true;

		loc.flush_rewrites();
		loc.set_route(pop_front(), dynamic_cast<location*>(parent));
		loc.set_modifier((front() == "=" ? pop_front() == "=" : false));
		while (front().at(0) != '}')
			parse(&loc);
		erase_chunk_front("}");

		static_cast<base_dir_ext*>(parent)->add_location(loc);

		_directives["limit_except"].second = false;
		return (parent);
	}

	void parser::load_base_dir()
	{
		_directives["root"].second = true;
		_directives["autoindex"].second = true;
		_directives["error_page"].second = true;
		_directives["client_max_body_size"].second = true;
		_directives["index"].second = true;
		_directives["cgi_executable"].second = true;
		_directives["cgi_extension"].second = true;
	}

	void parser::unload_base_dir()
	{
		_directives["cgi_extension"].second = false;
		_directives["cgi_executable"].second = false;
		_directives["index"].second = false;
		_directives["client_max_body_size"].second = false;
		_directives["error_page"].second = false;
		_directives["autoindex"].second = false;
		_directives["root"].second = false;
	}

	bool parser::read_root(base_dir *parent)
	{
		bool semicolon_erased = erase_chunk_middle(";");
		parent->set_root(pop_front());
		return (semicolon_erased);
	}

	bool parser::read_autoindex(base_dir *parent)
	{
		bool semicolon_erased = erase_chunk_middle(";");
		if (front() == "on")
			parent->set_autoindex(true);
		else if (front() == "off")
			parent->set_autoindex(false);
		else
			throw parsing_error("Invalid `autoindex` argument.");
		pop_front();
		return (semicolon_erased);
	}

	bool parser::read_error_page(base_dir *parent)
	{
		std::vector<unsigned int> response_codes;
		bool semicolon_erased = true;
		while (!erase_chunk_middle(";"))
		{
			const http_code status = static_cast<http_code>(ft::strtoul(front()));
			if (!(http::is_redirection_code(status) || http::is_error_code(status)))
			{
				semicolon_erased = false;
				break ;
			}
			response_codes.push_back(status);
			pop_front();
		}
		if (response_codes.empty())
			throw parsing_error("Error page syntax error.");
		for (size_t i = 0; i < response_codes.size(); i++)
			parent->add_error_page(response_codes[i], front());
		pop_front();
		return (semicolon_erased);
	}

	bool parser::read_client_max_body_size(base_dir *parent)
	{
		const char front_back = front()[front().length() - 1];
		bool semicolon_erased = erase_chunk_middle(";");
		int multiplier = 1; // default: bytes
		if (std::isalpha(front_back))
		{
			if (front_back == 'k' || front_back == 'K')
				multiplier = 1000;
			else if (front_back == 'm' || front_back == 'M')
				multiplier = 1000000;
			else if (front_back == 'g' || front_back == 'G')
				multiplier = 1000000000;
			else
				throw parsing_error("Bad storage unit extension.");
			front().erase(front().end() - 1); // front().pop_back(); in C++11
		}
		parent->set_client_max_body_size(multiplier * ft::strtoul(pop_front()));
		return (semicolon_erased);
	}

	bool parser::read_index(base_dir *parent)
	{
		std::vector<std::string> arguments = get_argument_list();
		std::cout << CYAN "parent in index is " RED << parent << RESET << std::endl;
		parent->flush_indices();
		std::cout << CYAN "STARTING ITERATION" RESET << std::endl;
		for (size_t i = 0; i < arguments.size(); i++)
			parent->add_index(arguments[i]);
		std::cout << CYAN "LEAVING INDEX" RESET << std::endl;
		return (true);
	}

	bool parser::read_rewrite(base_dir *parent)
	{
		bool semicolon_erased;
		std::string expr;
		if (erase_chunk_middle(";"))
			throw parsing_error("Invalid number of arguments for `rewrite`.");
		expr = pop_front();
		semicolon_erased = erase_chunk_middle(";");
		static_cast<base_dir_ext*>(parent)->add_rewrite(expr, pop_front());
		return (semicolon_erased);
	}

	bool parser::read_listen(base_dir*)
	{
		bool port_specified = erase_chunk_middle(":");
		bool semicolon_erased = false;
		std::string port = "80";
		std::string host = "0.0.0.0";
		if (!port_specified)
			semicolon_erased = erase_chunk_middle(";");
		if (!port_specified && http::is_port_number(front()))
			port = pop_front();
		else
			host = pop_front();
		if (port_specified)
		{
			semicolon_erased = erase_chunk_middle(";");
			port = pop_front();
		}
		memorize_listen(host, port);
		return (semicolon_erased);
	}

	bool parser::read_server_name(base_dir *parent)
	{
		std::vector<std::string> arguments = get_argument_list();
		for (size_t i = 0; i < arguments.size(); i++)
			static_cast<server*>(parent)->add_name(arguments[i]);
		return (true);
	}

	bool parser::read_cgi_executable(base_dir *parent)
	{
		bool semicolon_erased = erase_chunk_middle(";");
		parent->set_cgi_executable(pop_front());
		return (semicolon_erased);
	}

	bool parser::read_cgi_extension(base_dir *parent)
	{
		bool semicolon_erased = erase_chunk_middle(";");
		parent->set_cgi_extension(pop_front());
		return (semicolon_erased);
	}

	bool parser::read_limit_except(base_dir *loc)
	{
		std::vector<std::string> arguments = get_argument_list();
		static_cast<location*>(loc)->flush_methods();
		for (size_t i = 0; i < arguments.size(); i++)
			static_cast<location*>(loc)->add_method(arguments[i]);
		return (true);
	}

	std::string &parser::front()
	{
		if (_chunks.empty())
			throw parsing_error("Context or directive not completed.");
		return (_chunks.front());
	}

	std::string parser::pop_front()
	{
		std::string popped(front());
		_chunks.pop_front();
		return (popped);
	}

	// returns a vector of strings that are arguments delimited by a `;`
	string_vector parser::get_argument_list()
	{
		string_vector arguments;
		if (front().at(0) == ';')
			throw parsing_error("Empty argument list.");
		while (!erase_chunk_middle(";", true))
			arguments.push_back(pop_front());
		if (!front().empty())
			arguments.push_back(front());
		pop_front();
		return (arguments);
	}

	void parser::memorize_listen(const std::string &host, const std::string &port)
	{
		int	status;
		struct addrinfo hints;
		struct addrinfo *result, *rit;

		std::memset(&hints, 0, sizeof(struct addrinfo)); // clear hints structure
		hints.ai_protocol = IPPROTO_TCP;				 // only TCP connections
		hints.ai_family = AF_INET;						 // what family to search? ipv4
		hints.ai_socktype = SOCK_STREAM;				 // what type of _sockets?
		hints.ai_flags = AI_ADDRCONFIG;					 // only address families configured on the system
		if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result)) != 0)
			throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(status)));
		for (rit = result; rit != NULL; rit = rit->ai_next)
		{
			const std::string host = ft::inet_ntoa(reinterpret_cast<struct sockaddr_in*>(rit->ai_addr)->sin_addr);
			if (_listens.find(string_pair(host, port)) != _listens.end())
				throw parsing_error("A duplicate listen " + host + ":" + port + " encountered.");
			_listens.insert(string_pair(host, port));
		}
		freeaddrinfo(result);
	}

	void parser::map_sockets(const server *server)
	{
		if (_listens.empty())
			_sockets.insert(std::make_pair(string_pair("0.0.0.0", "80"), server));
		else
		{
			for (string_pair_set::iterator it = _listens.begin(); it != _listens.end(); it++)
				_sockets.insert(std::make_pair(string_pair(it->first, it->second), server));
			_listens.clear();
		}
	}

	void parser::open_sockets(http *protocol)
	{
		for (string_pair_server_pointer_mmap::iterator it = _sockets.begin(); it != _sockets.end();)
		{
			std::pair<string_pair_server_pointer_mmap::iterator, string_pair_server_pointer_mmap::iterator> range = _sockets.equal_range(it->first);
			const std::string host = it->first.first;
			const std::string port = it->first.second;

			int	status;
			struct addrinfo hints;
			struct addrinfo *result, *rit;

			std::memset(&hints, 0, sizeof(struct addrinfo)); // clear hints structure
			hints.ai_protocol = IPPROTO_TCP;				 // only TCP connections
			hints.ai_family = AF_INET;						 // what family to search? ipv4
			hints.ai_socktype = SOCK_STREAM;				 // what type of _sockets?
			hints.ai_flags = AI_ADDRCONFIG;					 // only address families configured on the system
			if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result)) != 0)
				throw std::runtime_error("[GETADDRINFO] " + std::string(gai_strerror(status)));
			rit = result;

			server_socket socket(::socket(rit->ai_family, rit->ai_socktype, rit->ai_protocol), host, port);
			if (socket == -1)
				throw std::runtime_error("[SOCKET] Failed to create a socket.");
			webserver.label_log("Successfully created a socket " + to_string(socket), BOLDED("SOCKET"), GREEN, GREEN);
			int on = 1;
			if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) == -1
				|| setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) == -1
				|| setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == -1)
			{
				close(socket);
				throw std::runtime_error("[SETSOCKETOPT] Couldn't set socket options.");
			}
			if (fcntl(socket, F_SETFL, O_NONBLOCK) == -1)
			{
				close(socket);
				throw std::runtime_error("[FCNTL] Couldn't set the flags of a socket.");
			}
			if (bind(socket, rit->ai_addr, rit->ai_addrlen) == -1)
			{
				close(socket);
				throw std::runtime_error("[BIND] bind() to " + host + ":" + port + " failed (" + strerror(errno) + ")");
			}
			webserver.label_log("Successfully bound to address " + host + ":" + port, BOLDED("BIND"), GREEN, GREEN);
			if (listen(socket, BACKLOG) == -1)
			{
				close(socket);
				throw std::runtime_error("[LISTEN] Failed listening on " + host + ":" + port + ".");
			}
			webserver.label_log("Started listening on address " + host + ":" + port, BOLDED("LISTEN"), GREEN, GREEN);
			freeaddrinfo(result);
			while (it != range.second)
			{
				socket.add_server(it->second);
				it++;
			}
			protocol->add_socket(socket);
		}
	}

	// returns true if the erased str was a separate chunk, otherwise false
	bool parser::erase_chunk_front(std::string str)
	{
		if (front().substr(0, str.length()) == str)
			front() = front().substr(str.length(), front().length() - str.length());
		if (front().empty())
		{
			pop_front();
			return (true);
		}
		return (false);
	}

	// returns true if str was removed, false if str wasn't found
	// splits the front chunk if str is in the middle
	// if there's nothing left before the deleted str, an exception is thrown, unless empty high is allowed
	bool parser::erase_chunk_middle(std::string str, bool empty_high_allowed)
	{
		const size_t str_index = front().find(str);
		if (str_index == std::string::npos)
			return (false);
		const std::string low = front().substr(str_index + 1);
		const std::string high = pop_front().substr(0, str_index);
		if (!empty_high_allowed && high.empty())
			throw parsing_error("Emptiness before a " + str + " token.");
		if (!low.empty())
			_chunks.push_front(low);
		_chunks.push_front(high);
		return (true);
	}

	void parser::erase_token_front(std::string token, bool erased_before)
	{
		if (!erased_before)
		{
			if (front().substr(0, token.length()) != token)
				throw parsing_error(token + " not met when expected.");
			erase_chunk_front(token);
		}
	}
}
