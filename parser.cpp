#include "parser.hpp"
#include "http.hpp"
#include "location.hpp"

parser::parser()
{
}

parser::~parser() {}

parser::parser(const parser &other) {}

parser &parser::operator=(const parser &other)
{
}

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
			(this->*(it->second))(parent);
	}
	if (!parent)
		throw std::invalid_argument("Parsing error occured.");
	return (parent);
}

bool parser::is_context(std::string context)
{
	std::vector<std::string> location_args;

	if (context.compare(0, context.length(), this->chunks.front()) == 0)
	{
		if (erase_chunk_front(context) && context == "location")
		{
			if (this->chunks.front() == "=")
			{
				location_args.push_back("=");
				this->chunks.pop_front();
			}
			const size_t brace_index = this->chunks.front().find("{");
			if (brace_index != std::string::npos)
			{
				location_args.push_back(this->chunks.front().substr(0, brace_index));
				this->chunks.front() = this->chunks.front().substr(brace_index, this->chunks.front().length() - brace_index);
			}
			else
			{
				location_args.push_back(this->chunks.front());
				this->chunks.pop_front();
			}
		}
		else if (context == "location")
			throw std::invalid_argument("Location parsing error occured.");
		if (this->chunks.front()[0] == '{')
			erase_chunk_front("{");
		else
			throw std::invalid_argument("Opening brace not found after the context.");
		for (size_t i = 0; i < location_args.size(); i++)
			this->chunks.push_front(location_args[i]);
		push_brace('{');
		return (true);
	}
	return (false);
}

bool parser::is_directive(std::string directive)
{
	if (directive.compare(0, directive.length(), this->chunks.front()))
		return (false);
	if (!erase_chunk_front(directive))
		throw std::invalid_argument("Invalid directive name.");
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
			if (chunk[0] == '#')
				buffer.str(""); // clear buffer
			else
				this->chunks.push_back(chunk);
		}
	}
}

// returns true if the erased str was a separate chunk, otherwise false
bool parser::erase_chunk_front(std::string str)
{
	if (!str.compare(0, str.length(), this->chunks.front()))
		this->chunks.front() = this->chunks.front().substr(str.length(), this->chunks.front().length() - str.length());
	if (this->chunks.front().empty())
	{
		this->chunks.pop_front();
		return (true);
	}
	return (false);
}

base_dir *parser::process_http(base_dir *)
{
	http *protocol = new http();
	this->contexts.erase("http");
	this->contexts.insert(context("server", &parser::process_server));
	load_base_dir();

	while (this->chunks.front()[0] != '}')
		parse(protocol);
	erase_chunk_front("}");
	push_brace('}');

	this->contexts.erase("server");
	unload_base_dir();
	return (protocol);
}

base_dir *parser::process_server(base_dir *protocol)
{
	server serv(*protocol);
	this->contexts.erase("server");
	this->contexts.insert(context("location", &parser::process_location));
	this->directives.insert(directive("listen", &parser::read_listen));
	this->directives.insert(directive("server_name", &parser::read_server_name));

	while (this->chunks.front()[0] != '}')
		parse(&serv);
	erase_chunk_front("}");
	push_brace('}');

	static_cast<http *>(protocol)->add_server(serv);

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

	while (this->chunks.front()[0] != '}')
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

void parser::read_client_max_body_size(base_dir *parent)
{
	std::string str;

	size_t index = this->chunks.front().find(";");
	if (index == std::string::npos)
		str = this->chunks.front(); // am I allowed to use the function?
	else
		str = this->chunks.front().substr(0, index);
	erase_chunk_front(str);
	if (this->chunks.front()[0] == ';') // if there is no chunks after the directive, throws exception
		erase_chunk_front(";");
	else
		throw std::invalid_argument("Parsing error occured. No ; after the directive.");
	parent->set_client_max_body_size(std::strtol(str.c_str(), NULL, 10));
}

void parser::read_index(base_dir *parent)
{
	size_t index = this->chunks.front().find(";");
	while (index == std::string::npos)
	{
		parent->add_index(this->chunks.front());
		erase_chunk_front(this->chunks.front());
		index = this->chunks.front().find(";");
	}
	parent->add_index(this->chunks.front().substr(0, index));
	erase_chunk_front(this->chunks.front().substr(0, index));
	if (this->chunks.front()[0] == ';') // is the if necessary?
		erase_chunk_front(";");
	else
		throw std::invalid_argument("Parsing error occured. No ; after the directive.");
}

void parser::read_redirect(base_dir *parent)
{
	std::string expr;
	bool semicolon;

	if (erase_chunk_middle(";"))
		throw std::invalid_argument("Parsing error.");
	else
	{
		expr = this->chunks.front();
		this->chunks.pop_front();
	}
	semicolon = erase_chunk_middle(";");
	if (this->chunks.front().empty())
		throw std::invalid_argument("Parsing error.");
	static_cast<base_dir_ext *>(parent)->add_redirect(expr, this->chunks.front());
	if (!semicolon && this->chunks.front()[0] == ';')
		erase_chunk_front(";");
	else
		throw std::invalid_argument("Parsing error.");
}

void parser::read_root(base_dir *parent)
{
	bool semicolon = erase_chunk_middle(";");
	// if (this->chunks.front().empty()) ???
	parent->set_root(this->chunks.front());
	this->chunks.pop_front();
	if (!semicolon && this->chunks.front()[0] != ';')
		throw std::invalid_argument("Parsing error occured. No ; after the directive.");
	erase_chunk_front(";"); // in case erase_chunk_middle() didn't remove the ;
	// blabla ; root ...  => "blabla", ";", "root"       semicolon = false > SUCCESS
	// blabla ;root ... => "blabla", ";root"       semicolon = false   >> SUCCESS
	// blabla; root  => "blabla", "root" ...      semicolon = true    >> SUCCESS
	// blabla;root   => "blabla", "root", ... semicolon = true       >> SUCCESS
	// ; root        => "", "root"              semicolon = true     >>
	// ;root         => "", "root"              semicolon = true     >>
	// blabla root   => "blabla", "root"       semicolon = false   >> FALSE
}

void parser::read_autoindex(base_dir *parent)
{
	bool semicolon = erase_chunk_middle(";");
	if (this->chunks.front() == "on")
		parent->set_autoindex(true);
	else if (this->chunks.front() == "off")
		parent->set_autoindex(false);
	else
		throw std::invalid_argument("Parsing error occured. Invalid autoindex argument.");
	this->chunks.pop_front();
	if (!semicolon && this->chunks.front()[0] != ';')
		throw std::invalid_argument("Parsing error occured. No ; after the directive.");
	erase_chunk_front(";");
}

void parser::read_error_page(base_dir *parent)
{
	// 404 421 132 433 111 ... path;
	// 404 421 132 433 111 ... path ; root
	// 404 421 132 433 111 ... path ;root
	// 404 421 132 433 111 ... path;root
	// path; => "path"  semicolon = true
	// 404 421 132 433 111 ... path;
	// 
	std::vector<unsigned int> response_codes;
	while (!erase_chunk_middle(";"))
	{
		const unsigned int response_code = strtol(this->chunks.front().c_str(), nullptr, 10);
		if (!is_response_code(response_code))
			break ;
		response_codes.push_back(response_code);
		this->chunks.pop_front();
	}
	if (response_codes.empty() || this->chunks.front().empty());
		throw std::invalid_argument("Parsing error occured. Error page syntax error.");
	for (int i = 0; i < response_codes.size(); i++)
		parent->add_error_page(response_codes[i], this->chunks.front());
	this->chunks.pop_front();
	erase_chunk_front(";");
}

// address:port;      => "address", "port;"
// address:port;root  => "address", "port;root"
// address:port ;     => "address", "port", ";"
// ;
// address;			  => "address;"
// address ;		  => "address", ";"
// address ;root      => "address", ";root"
// address;root       => "address;root"
void parser::read_listen(base_dir *parent)
{
	bool port_specified = erase_chunk_middle(":");
	if (this->chunks.front().empty())
		throw std::invalid_argument("Parsing error occured. Listen syntax error.");
	std::string address;
	unsigned int port = 80;
	if (!port_specified)
		erase_chunk_middle(";");
	address = this->chunks.front();
	this->chunks.pop_front();
	if (port_specified)
	{
		erase_chunk_middle(";");
		// port = 
	}
	erase_chunk_front(";");
}

bool parser::is_response_code(unsigned int response_code)
{
	return (response_code >= 100 && response_code < 600);
}

// returns true if str was removed, false if str wasn't found
// splits the front chunk if str is in the middle
bool parser::erase_chunk_middle(std::string str)
{
	const std::string front = this->chunks.front();
	const size_t str_index = front.find(str);
	if (str_index == std::string::npos)
		return (false);
	const std::string low = front.substr(str_index + 1);
	const std::string high = front.substr(0, str_index);
	this->chunks.pop_front();
	if (!low.empty())
		this->chunks.push_front(low);
	this->chunks.push_front(high);
	return (true);
}

unsigned int parser::strtoui(const std::string &number)
{
	const char *str_begin = number.c_str();
	char *str_end = nullptr;
	if (!isdigit(number[0]))
		throw std::invalid_argument("Number parsing error occured.");
	unsigned int ui = strtoul(str_begin, &str_end, 10);
	if (str_end != str_begin + number.length() || errno == ERANGE)
		throw std::invalid_argument("Number parsing error occured.");
	return (ui);
}

void parser::read_limit_except(base_dir *loc)
{
	if (this->chunks.front()[0] == ';')
		throw std::invalid_argument("Parsing error. Few arguments for the limit_except directive.");
	while (!erase_chunk_middle(";"))
	{
		static_cast<location *>(loc)->add_method(this->chunks.front());
		this->chunks.pop_front();
	}
	if (!this->chunks.front().empty())
	{
		erase_chunk_middle(";");
		static_cast<location *>(loc)->add_method(this->chunks.front());
	}
	this->chunks.pop_front();
}

void parser::read_cgi(base_dir *loc)
{
	std::string ext;
	std::string path;

	if (!erase_chunk_middle(";"))
	{
		ext = this->chunks.front();
		this->chunks.pop_front();
		if (!erase_chunk_middle(";"))
		{
			path = this->chunks.front();
			this->chunks.pop_front();
			if (this->chunks.front()[0] == ';')
				erase_chunk_front(";");
			else
				throw std::invalid_argument("Parsing error! No ; in the end of directive.");
		}
		else
		{
			path = this->chunks.front();
			this->chunks.pop_front();
		}
	}
	else
		throw std::invalid_argument("Parsing error! Few arguments for cgi directive.");
	static_cast<location *>(loc)->add_cgi(ext, path);
}
