#include "parser.hpp"
#include "http.hpp"
#include "location.hpp"
#include <iostream> //remove later

namespace ft
{
	parser::parser(): directives(), contexts(), braces(), config(), chunks() {}

	parser::parser(const std::string &filename): directives(), contexts(), braces(), config(), chunks() 
	{
		this->config.open(filename);
		if (!this->config.is_open())
				throw std::runtime_error("File does not exist.");
		this->contexts.insert(context("http", &parser::process_http));
		get_chunks();
	}

	parser::~parser() {}

	parser::parser(const parser &other): directives(other.directives), contexts(other.contexts), braces(other.braces), config(), chunks(other.chunks) {}

	parser &parser::operator=(const parser&) { return (*this); }

	base_dir *parser::parse(base_dir *parent)
	{
		for (context_map::iterator it = this->contexts.begin(); it != this->contexts.end(); it++)
		{
			if (is_context(it->first))
				parent = (this->*(it->second))(parent);
		}
		for (directive_map::iterator it = this->directives.begin(); it != this->directives.end(); it++)
		{
			if (is_directive(it->first))
				erase_token_front(";", (this->*(it->second))(parent));
		}
		if (!parent && !this->braces.empty())
			throw std::logic_error("Ill-formed context. Braces not closed!");
		return (parent);
	}

	bool parser::is_context(std::string context)
	{
		std::vector<std::string> location_args;

		if (!context.compare(0, context.length(), front()))
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
				throw std::invalid_argument("Parsing error occured. Invalid `location` syntax.");
			erase_token_front("{", brace_erased);
			for (size_t i = 0; i < location_args.size(); i++)
				this->chunks.push_front(location_args[i]);
			push_brace('{');
			return (true);
		}
		return (false);
	}

	bool parser::is_directive(std::string directive)
	{
		if (directive.compare(0, directive.length(), front()))
			return (false);
		if (!erase_chunk_front(directive))
			throw std::invalid_argument("Parsing error occured. Unknown directive provided.");
		return (true);
	}

	void parser::get_chunks()
	{
		std::string line;

		while (std::getline(this->config, line))
		{
			std::string chunk;
			std::stringstream buffer(line);
			while (buffer >> chunk)
			{
				if (chunk.at(0) == '#')
					buffer.str(""); // clear buffer
				else
					this->chunks.push_back(chunk);
			}
		}
		
		//remove later
		std::cout << "Printing chunks\n";
		for (std::list<std::string>::iterator it = this->chunks.begin(); it != this->chunks.end(); it++)
			std::cout << *it << std::endl;
	}

	base_dir *parser::process_http(base_dir*)
	{
		http *protocol = new http();
		this->contexts.erase("http");
		this->contexts.insert(context("server", &parser::process_server));
		load_base_dir();

		while (front().at(0) != '}')
			parse(protocol);
		erase_chunk_front("}");
		push_brace('}');

		unload_base_dir();
		this->contexts.erase("server");
		return (protocol);
	}

	base_dir *parser::process_server(base_dir *protocol)
	{
		server serv(*protocol);
		this->contexts.erase("server");
		this->contexts.insert(context("location", &parser::process_location));
		this->directives.insert(directive("listen", &parser::read_listen));
		this->directives.insert(directive("server_name", &parser::read_server_name));
		this->directives.insert(directive("rewrite", &parser::read_redirect));

		while (front().at(0) != '}')
			parse(&serv);
		erase_chunk_front("}");
		push_brace('}');

		static_cast<http *>(protocol)->add_server(serv);

		this->directives.erase("rewrite");
		this->directives.erase("server_name");
		this->directives.erase("listen");
		this->contexts.erase("location");
		this->contexts.insert(context("server", &parser::process_server));
		return (protocol);
	}

	base_dir *parser::process_location(base_dir *parent)
	{
		location loc(*parent);
		this->directives.insert(directive("cgi", &parser::read_cgi));
		this->directives.insert(directive("limit_except", &parser::read_limit_except));

		while (front().at(0) != '}')
			parse(&loc);
		erase_chunk_front("}");
		push_brace('}');

		static_cast<base_dir_ext *>(parent)->add_location(loc);

		this->directives.erase("limit_except");
		this->directives.erase("cgi");
		return (parent);
	}

	void parser::load_base_dir()
	{
		this->directives.insert(directive("root", &parser::read_root));
		this->directives.insert(directive("autoindex", &parser::read_autoindex));
		this->directives.insert(directive("error_page", &parser::read_error_page));
		this->directives.insert(directive("client_max_body_size", &parser::read_client_max_body_size));
		this->directives.insert(directive("index", &parser::read_index));
	}

	void parser::unload_base_dir()
	{
		this->directives.erase("index");
		this->directives.erase("client_max_body_size");
		this->directives.erase("error_page");
		this->directives.erase("autoindex");
		this->directives.erase("root");
	}

	void parser::push_brace(char brace)
	{
		if (brace != '{' && brace != '}')
			return;
		if (brace == '}')
		{
			if (this->braces.top() == '{')
				this->braces.pop();
		}
		this->braces.push(brace);
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
			throw std::invalid_argument("Parsing error occured. Invalid `autoindex` argument.");
		pop_front();
		return (semicolon_erased);
	}

	bool parser::read_error_page(base_dir *parent)
	{
		std::vector<unsigned int> response_codes;
		bool semicolon_erased = true;
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
			throw std::invalid_argument("Parsing error occured. Error page syntax error.");
		for (size_t i = 0; i < response_codes.size(); i++)
			parent->add_error_page(response_codes[i], front());
		pop_front();
		return (semicolon_erased);
	}

	bool parser::read_client_max_body_size(base_dir *parent)
	{
		bool semicolon_erased = erase_chunk_middle(";");
		int multiplier = 1; // default: bytes

		if (front().front() == '-')
			throw std::invalid_argument("Parsing error occured. Negative `client_max_body_size` is disallowed.");
		front().back() = std::tolower(front().back());
		if (front().back() == 'k')
			multiplier = 1000;
		else if (front().back() == 'm')
			multiplier = 1000000;
		else if (front().back() == 'g')
			multiplier = 1000000000;
		if (!std::isdigit(front().back()))
			front().pop_back();
		parent->set_client_max_body_size(multiplier * strtoul(pop_front()));
		return (semicolon_erased);
	}

	bool parser::read_index(base_dir *parent)
	{
		std::vector<std::string> arguments = get_argument_list();
		for (size_t i = 0; i < arguments.size(); i++)
			parent->add_index(arguments[i]);
		return (true);
	}

	bool parser::read_redirect(base_dir *parent)
	{
		std::string expr;
		bool semicolon_erased;

		if (erase_chunk_middle(";"))
			throw std::invalid_argument("Parsing error occured. Invalid number of arguments for `rewrite`.");
		expr = pop_front();
		semicolon_erased = erase_chunk_middle(";");
		static_cast<base_dir_ext*>(parent)->add_redirect(expr, pop_front());
		return (semicolon_erased);
	}

	bool parser::read_listen(base_dir *parent)
	{
		bool port_specified = erase_chunk_middle(":");
		bool semicolon_erased = false;
		std::string host;
		unsigned int port = 80;
		if (!port_specified)
			semicolon_erased = erase_chunk_middle(";");
		host = pop_front();
		if (port_specified)
		{
			semicolon_erased = erase_chunk_middle(";");
			port = strtoul(pop_front());
		}
		static_cast<server*>(parent)->add_listen(host, port);
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

		if (!erase_chunk_middle(";"))
		{
			extension = pop_front();
			semicolon_erased = erase_chunk_middle(";");
			path = pop_front();
		}
		else
			throw std::invalid_argument("Parsing error occured. Few arguments for `cgi` directive.");
		static_cast<location*>(loc)->add_cgi(extension, path);
		return (semicolon_erased);
	}

	bool parser::read_limit_except(base_dir *loc)
	{
		std::vector<std::string> arguments = get_argument_list();
		for (size_t i = 0; i < arguments.size(); i++)
			static_cast<location*>(loc)->add_method(arguments[i]);
		return (true);
	}

	// returns a vector of strings that are arguments delimited by a `;`
	std::vector<std::string> parser::get_argument_list()
	{
		std::vector<std::string> arguments;
		if (front().at(0) == ';')
			throw std::invalid_argument("Parsing error occured. Empty argument list.");
		while (!erase_chunk_middle(";", true))
			arguments.push_back(pop_front());
		if (!front().empty())
			arguments.push_back(front());
		pop_front();
		return (arguments);
	}

	unsigned int parser::strtoul(const std::string &number)
	{
		if (number.empty())
			throw std::invalid_argument("Number parsing error occured.");
		const char *str_begin = number.c_str();
		char *str_end = NULL;
		unsigned long long int ul = std::strtoul(str_begin, &str_end, 10);
		if (str_end != str_begin + number.length() || ul > std::numeric_limits<unsigned long int>::max())
			throw std::invalid_argument("Number parsing error occured.");
		return (ul);
	}

	std::string &parser::front()
	{
		if (this->chunks.empty())
			throw std::length_error("Ran out of chunks.");
		return (this->chunks.front());
	}

	std::string parser::pop_front()
	{
		std::string popped(front());
		this->chunks.pop_front();
		return (popped);
	}

	// returns true if the erased str was a separate chunk, otherwise false
	bool parser::erase_chunk_front(std::string str)
	{
		if (!str.compare(0, str.length(), front()))
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
		if (!empty_high_allowed)
			throw std::invalid_argument("Parsing error occured. Emptiness before a " + str + "token.");
		const std::string high = pop_front().substr(0, str_index);
		if (!low.empty())
			this->chunks.push_front(low);
		this->chunks.push_front(high);
		return (true);
	}

	void parser::erase_token_front(std::string token, bool erased_before)
	{
		if (!erased_before)
		{
			if (front().compare(0, token.length(), token))
				throw std::invalid_argument("Parsing error occured. " + token + " not met when expected.");
			erase_chunk_front(token);
		}
	}

	bool parser::is_response_code(unsigned int response_code)
	{
		return (response_code >= 300 && response_code < 600);
	}
}
