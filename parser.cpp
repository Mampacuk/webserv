#include "parser.hpp"
#include "http.hpp"
#include "location.hpp"
#include "server.hpp"

namespace ft
{
	parser::parser(): _directives(), _contexts(), _config(), _chunks() {}

	parser::parser(const std::string &filename): _directives(), _contexts(), _config(), _chunks()
	{
		this->_config.open(filename.c_str());
		if (!this->_config.is_open())
			throw std::runtime_error("File \"" + filename + "\" does not exist.");
		this->_contexts.insert(context("http", cont_functor(&parser::process_http, true)));
		this->_contexts.insert(context("server", cont_functor(&parser::process_server, false)));
		this->_contexts.insert(context("location", cont_functor(&parser::process_location, false)));
		this->_directives.insert(directive("root", dir_functor(&parser::read_root, false)));
		this->_directives.insert(directive("autoindex", dir_functor(&parser::read_autoindex, false)));
		this->_directives.insert(directive("error_page", dir_functor(&parser::read_error_page, false)));
		this->_directives.insert(directive("client_max_body_size", dir_functor(&parser::read_client_max_body_size, false)));
		this->_directives.insert(directive("index", dir_functor(&parser::read_index, false)));
		this->_directives.insert(directive("listen", dir_functor(&parser::read_listen, false)));
		this->_directives.insert(directive("server_name", dir_functor(&parser::read_server_name, false)));
		this->_directives.insert(directive("rewrite", dir_functor(&parser::read_redirect, false)));
		this->_directives.insert(directive("cgi", dir_functor(&parser::read_cgi, false)));
		this->_directives.insert(directive("limit_except", dir_functor(&parser::read_limit_except, false)));
		parse_chunks();
	}

	parser::~parser() {}

	parser::parser(const parser &other): _directives(other._directives), _contexts(other._contexts), _config(), _chunks(other._chunks) {}

	parser &parser::operator=(const parser&) { return (*this); }

	base_dir *parser::parse(base_dir *parent)
	{
		context_map::iterator cont_it = this->_contexts.begin();
		for (; cont_it != this->_contexts.end(); cont_it++)
			if (cont_it->second.second && is_context(cont_it->first))
			{
				parent = (this->*(cont_it->second.first))(parent);
				break ;
			}
		directive_map::iterator dir_it = this->_directives.begin();
		for (; dir_it != this->_directives.end(); dir_it++)
			if (dir_it->second.second && is_directive(dir_it->first))
			{
				erase_token_front(";", (this->*(dir_it->second.first))(parent));
				break ;
			}
		if (cont_it == this->_contexts.end() && dir_it == this->_directives.end() && !this->_chunks.empty())
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
				this->_chunks.push_front(location_args[i]);
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

		while (std::getline(this->_config, line))
		{
			std::string chunk;
			std::stringstream buffer(line);
			while (buffer >> chunk)
			{
				if (chunk.find('#') != std::string::npos)
				{
					this->_chunks.push_front(chunk);
					erase_chunk_middle("#", true);
					front().empty() ? (void)pop_front() : this->_chunks.push_back(pop_front());
					pop_front();
					buffer.str(""); // clear buffer
				}
				else
					this->_chunks.push_back(chunk);
			}
		}
	}

	base_dir *parser::process_http(base_dir*)
	{	
		http *protocol = new http();

		try
		{
			this->_contexts["http"].second = false;
			this->_contexts["server"].second = true;
			load_base_dir();
			while (front().at(0) != '}')
				parse(protocol);
			erase_chunk_front("}");
			unload_base_dir();
			this->_contexts["server"].second = false;
			parse(protocol);
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
		server serv(*protocol);
		this->_contexts["server"].second = false;
		this->_contexts["location"].second = true;
		this->_directives["listen"].second = true;
		this->_directives["server_name"].second = true;
		this->_directives["rewrite"].second = true;

		while (front().at(0) != '}')
			parse(&serv);
		erase_chunk_front("}");

		if (serv.get_sockets().empty())
			serv.add_socket("0.0.0.0", "80");
		if (serv.get_names().empty())
			serv.add_name("");
		open_sockets(serv);
		
		static_cast<http*>(protocol)->add_server(serv);

		this->_directives["rewrite"].second = false;
		this->_directives["server_name"].second = false;
		this->_directives["listen"].second = false;
		this->_contexts["location"].second = false;
		this->_contexts["server"].second = true;
		return (protocol);
	}

	base_dir *parser::process_location(base_dir *parent)
	{
		location loc(*parent);
		this->_directives["cgi"].second = true;
		this->_directives["limit_except"].second = true;

		loc.set_route(pop_front(), dynamic_cast<location*>(parent));
		loc.set_modifier((front() == "=" ? pop_front() == "=" : false));
		while (front().at(0) != '}')
			parse(&loc);
		erase_chunk_front("}");

		static_cast<base_dir_ext*>(parent)->add_location(loc);

		this->_directives["limit_except"].second = false;
		this->_directives["cgi"].second = false;
		return (parent);
	}

	void parser::load_base_dir()
	{
		this->_directives["root"].second = true;
		this->_directives["autoindex"].second = true;
		this->_directives["error_page"].second = true;
		this->_directives["client_max_body_size"].second = true;
		this->_directives["index"].second = true;
	}

	void parser::unload_base_dir()
	{
		this->_directives["index"].second = false;
		this->_directives["client_max_body_size"].second = false;
		this->_directives["error_page"].second = false;
		this->_directives["autoindex"].second = false;
		this->_directives["root"].second = false;
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
		parent->flush_error_pages();
		while (!erase_chunk_middle(";"))
		{
			const unsigned int code = strtoul(front());
			if (!is_response_code(code))
			{
				semicolon_erased = false;
				break ;
			}
			response_codes.push_back(code);
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
		parent->set_client_max_body_size(multiplier * strtoul(pop_front()));
		return (semicolon_erased);
	}

	bool parser::read_index(base_dir *parent)
	{
		std::vector<std::string> arguments = get_argument_list();
		parent->flush_indexes();
		for (size_t i = 0; i < arguments.size(); i++)
			parent->add_index(arguments[i]);
		return (true);
	}

	bool parser::read_redirect(base_dir *parent)
	{
		bool semicolon_erased;
		std::string expr;
		if (erase_chunk_middle(";"))
			throw parsing_error("Invalid number of arguments for `rewrite`.");
		expr = pop_front();
		semicolon_erased = erase_chunk_middle(";");
		static_cast<base_dir_ext*>(parent)->add_redirect(expr, pop_front());
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
		if (!port_specified && is_port_number(front()))
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

	bool parser::read_cgi(base_dir *loc)
	{
		std::string extension;
		std::string path;
		bool semicolon_erased = false;
		static_cast<location*>(loc)->flush_cgi();
		if (!erase_chunk_middle(";"))
		{
			extension = pop_front();
			semicolon_erased = erase_chunk_middle(";");
			path = pop_front();
		}
		else
			throw parsing_error("Few arguments for `cgi` directive.");
		static_cast<location*>(loc)->add_cgi(extension, path);
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

	// doesn't tolerate leading or trailing whitespaces,
	// or characters in the middle of the number.
	unsigned int parser::strtoul(const std::string &number, int base)
	{
		if (number.empty() || number[0] == '-' || std::isspace(number[0]) || base < 2)
			throw parsing_error("Number parsing error occured.");
		const char *str_begin = number.c_str();
		char *str_end = NULL;
		unsigned long long int ul = std::strtoul(str_begin, &str_end, base);
		if (str_end != str_begin + number.length() || ul > std::numeric_limits<unsigned int>::max())
			throw parsing_error("Number parsing error occured.");
		return (ul);
	}

	std::string &parser::front()
	{
		if (this->_chunks.empty())
			throw parsing_error("Context or directive not completed.");
		return (this->_chunks.front());
	}

	std::string parser::pop_front()
	{
		std::string popped(front());
		this->_chunks.pop_front();
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
		if (this->_listens.find(string_pair(host, port)) != this->_listens.end())
			throw parsing_error("A duplicate listen " + host + ":" + port + " encountered.");
		this->_listens.insert(string_pair(host, port));
	}

	void parser::open_sockets(server &server)
	{
		for (string_pair_set::iterator it = this->_listens.begin();
			it != this->_listens.end();
			it++)
		{
			server.add_socket(it->first, it->second);
		}
		this->_listens.clear();
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
			this->_chunks.push_front(low);
		this->_chunks.push_front(high);
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

	bool parser::is_response_code(unsigned int response_code)
	{
		return (response_code >= 300 && response_code < 600);
	}

	bool parser::is_port_number(const std::string &port_string)
	{
		try
		{
			const unsigned int port = strtoul(port_string);
			return (port >= 1 && port <= 65535);
		}
		catch (const std::exception &e)
		{
			return (false);
		}
	}

	bool ends_with(const std::string &str, const std::string &suffix)
	{
		return (str.size() >= suffix.size() && !str.compare(str.size() - suffix.size(), std::string::npos, suffix));
	}
}
