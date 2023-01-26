#include "parser.hpp"
#include "http.hpp"

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
		if (erase_chunk_front(context))
		{
			if (context == "location")
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
			if (this->chunks.front()[0] == '{')
				erase_chunk_front("{");
			else
				throw std::invalid_argument("Opening brace not found after the context.");
			for (size_t i = 0; i < location_args.size(); i++)
				this->chunks.push_front(location_args[i]);
			push_brace('{');
			return (true);
		}
		else
			throw std::invalid_argument("Location parsing error occured.");
	}
	return (false);
}

bool parser::is_directive(std::string directive)
{
	if (directive.compare(0, directive.length(), this->chunks.front()))
		return (false);
	erase_chunk_front(directive);
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

bool parser::erase_chunk_front(std::string str)
{
	if (!str.compare(0, str.length(), this->chunks.front()))
		this->chunks.front() = this->chunks.front().substr(str.length(), this->chunks.front().length() - str.length());
	if (this->chunks.front().empty())
	{
		this->chunks.pop_front();
		if (str == "location")
			return (false);		// only indicates failure of "location" context error!
	}
	return (true);
}

base_dir *parser::process_http(base_dir*)
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
	server	serv(*protocol);
	this->contexts.erase("server");
	this->contexts.insert(context("location", &parser::process_location));
	this->directives.insert(directive("listen", &parser::read_listen));
	this->directives.insert(directive("server_name", &parser::read_server_name));

	while (this->chunks.front()[0] != '}')
		parse(&serv);
	erase_chunk_front("}");
	push_brace('}');

	static_cast<http*>(protocol)->add_server(serv);

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

	static_cast<base_dir_ext*>(parent)->add_location(loc);

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
		return ;
	if (brace == '}')
	{
		if (this->braces.top() == '{')
			this->braces.pop();
	}
	this->braces.push(brace);
}