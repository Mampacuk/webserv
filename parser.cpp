#include "parser.hpp"
#include "http.hpp"
#include "location.hpp"
#include <iostream> //remove later

namespace ft
{
	parser::parser(): directives(), contexts(), config(), chunks() {}

	parser::parser(const std::string &filename): directives(), contexts(), config(), chunks()
	{
		this->config.open(filename.c_str());
		if (!this->config.is_open())
			throw std::runtime_error("File does not exist.");
		this->contexts.insert(context("http", cont_functor(&parser::process_http, true)));
		this->contexts.insert(context("server", cont_functor(&parser::process_server, false)));
		this->contexts.insert(context("location", cont_functor(&parser::process_location, false)));
		this->directives.insert(directive("root", dir_functor(&parser::read_root, false)));
		this->directives.insert(directive("autoindex", dir_functor(&parser::read_autoindex, false)));
		this->directives.insert(directive("error_page", dir_functor(&parser::read_error_page, false)));
		this->directives.insert(directive("client_max_body_size", dir_functor(&parser::read_client_max_body_size, false)));
		this->directives.insert(directive("index", dir_functor(&parser::read_index, false)));
		this->directives.insert(directive("listen", dir_functor(&parser::read_listen, false)));
		this->directives.insert(directive("server_name", dir_functor(&parser::read_server_name, false)));
		this->directives.insert(directive("rewrite", dir_functor(&parser::read_redirect, false)));
		this->directives.insert(directive("cgi", dir_functor(&parser::read_cgi, false)));
		this->directives.insert(directive("limit_except", dir_functor(&parser::read_limit_except, false)));
		get_chunks();
	}

	parser::~parser() {}

	parser::parser(const parser &other): directives(other.directives), contexts(other.contexts), config(), chunks(other.chunks) {}

	parser &parser::operator=(const parser&) { return (*this); }

	base_dir *parser::parse(base_dir *parent)
	{
		context_map::iterator cont_it = this->contexts.begin();
		for (; cont_it != this->contexts.end(); cont_it++)
			if (cont_it->second.second && is_context(cont_it->first))
			{
				parent = (this->*(cont_it->second.first))(parent);
				break ;
			}
		directive_map::iterator dir_it = this->directives.begin();
		for (; dir_it != this->directives.end(); dir_it++)
			if (dir_it->second.second && is_directive(dir_it->first))
			{
				erase_token_front(";", (this->*(dir_it->second.first))(parent));
				break ;
			}
		if (cont_it == this->contexts.end() && dir_it == this->directives.end() && !this->chunks.empty())
			throw std::logic_error("Unknown directive or ill-formed context encountered.");
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
				throw std::invalid_argument("Parsing error occured. Invalid `location` syntax.");
			erase_token_front("{", brace_erased);
			for (size_t i = 0; i < location_args.size(); i++)
				this->chunks.push_front(location_args[i]);
			return (true);
		}
		return (false);
	}

	bool parser::is_directive(std::string directive)
	{
		if (front().substr(0, directive.length()) != directive)
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
				if (chunk.at(0) == '#') // ? quotes and escaping? # in the middle?
					buffer.str(""); // clear buffer
				else
					this->chunks.push_back(chunk);
			}
		}

		//remove later
		// std::cout << ">>>>>>>>>>>>>>>>>>> Printing chunks <<<<<<<<<<<<<<<<<<<" << std::endl;
		// for (std::list<std::string>::iterator it = this->chunks.begin(); it != this->chunks.end(); it++)
		// 	std::cout << *it << std::endl;
		// std::cout << ">>>>>>>>>>>>>>>>>>> Finished chunks <<<<<<<<<<<<<<<<<<<" << std::endl;
	}

	base_dir *parser::process_http(base_dir*)
	{	
		http *protocol = new http();

		try
		{
			this->contexts["http"].second = false;
			this->contexts["server"].second = true;
			load_base_dir();
			while (front().at(0) != '}')
				parse(protocol);
			erase_chunk_front("}");
			unload_base_dir();
			this->contexts["server"].second = false;
			parse(protocol);
		}
		catch (const std::exception &e)
		{
			delete protocol;
			throw ;
		}
		return (protocol);
	}

	base_dir *parser::process_server(base_dir *protocol)
	{
		std::cout << "vv process_server vv" << std::endl;

		server serv(*protocol);
		this->contexts["server"].second = false;
		this->contexts["location"].second = true;
		this->directives["listen"].second = true;
		this->directives["server_name"].second = true;
		this->directives["rewrite"].second = true;

		while (front().at(0) != '}')
			parse(&serv);
		erase_chunk_front("}");

		if (serv.get_listens().empty())
			serv.add_listen("0.0.0.0");
		static_cast<http*>(protocol)->add_server(serv);

		this->directives["rewrite"].second = false;
		this->directives["server_name"].second = false;
		this->directives["listen"].second = false;
		this->contexts["location"].second = false;
		this->contexts["server"].second = true;
		std::cout << "^^ process_server ^^" << std::endl;
		return (protocol);
	}

	base_dir *parser::process_location(base_dir *parent)
	{
		std::cout << "vvv process_location vvv" << std::endl;

		std::cout << "dynamic cast succeeded? " << (dynamic_cast<location*>(parent) ? "yes" : "no") << std::endl;
		location loc(*(dynamic_cast<location*>(parent) ? dynamic_cast<location*>(parent) : parent));
		this->directives["cgi"].second = true;
		this->directives["limit_except"].second = true;

		loc.set_route(pop_front());
		loc.set_modifier((front() == "=" ? pop_front() == "=" : false));
		while (front().at(0) != '}')
			parse(&loc);
		erase_chunk_front("}");

		static_cast<base_dir_ext*>(parent)->add_location(loc);

		this->directives["limit_except"].second = false;
		this->directives["cgi"].second = false;
		std::cout << "^^^ process_location ^^^" << std::endl;
		return (parent);
	}

	void parser::load_base_dir()
	{
		this->directives["root"].second = true;
		this->directives["autoindex"].second = true;
		this->directives["error_page"].second = true;
		this->directives["client_max_body_size"].second = true;
		this->directives["index"].second = true;
	}

	void parser::unload_base_dir()
	{
		this->directives["index"].second = false;
		this->directives["client_max_body_size"].second = false;
		this->directives["error_page"].second = false;
		this->directives["autoindex"].second = false;
		this->directives["root"].second = false;
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
			throw std::invalid_argument("Parsing error occured. Error page syntax error.");
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
				throw std::invalid_argument("Parsing error occured. Bad storage unit extension.");
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
		unsigned int port = 80;
		std::string host;
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
		// static_cast<location*>(loc)->flush_cgi(); // should we flush?
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
		static_cast<location*>(loc)->flush_methods();
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
		if (number.empty() || number[0] == '-')
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
			throw std::length_error("Context or directive not completed.");
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
			throw std::invalid_argument("Parsing error occured. Emptiness before a " + str + " token.");
		if (!low.empty())
			this->chunks.push_front(low);
		this->chunks.push_front(high);
		return (true);
	}

	void parser::erase_token_front(std::string token, bool erased_before)
	{
		if (!erased_before)
		{
			if (front().substr(0, token.length()) != token)
				throw std::invalid_argument("Parsing error occured. " + token + " not met when expected.");
			erase_chunk_front(token);
		}
	}

	bool parser::is_response_code(unsigned int response_code)
	{
		return (response_code >= 300 && response_code < 600);
	}
}
