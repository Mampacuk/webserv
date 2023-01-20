#include "parser.hpp"

parser::parser()
{

}

parser::~parser() {}

parser::parser(const parser &other) {}

parser &parser::operator=(const parser &other)
{

}

// void parser::get_chunks()
// {
// 	std::string line;
// 	if ()
// }

base_dir *parser::parse(base_dir *parent)
{
	for (context_map::iterator it = this->contexts.begin(); it != this->contexts.end(); it++)
	{
		if (is_context(it->first))
		{
			this->braces.push('{');
			parent = (this->*(it->second))(parent);
		}
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
	if (context.compare(0, context.length(), this->chunks.front()) == 0)
	{
		this->chunks.front() = this->chunks.front().substr(context.length(), this->chunks.front().length() - context.length());
		if (context != "location")
		{
			if (this->chunks.front().empty())
				this->chunks.pop_front();
			return (this->chunks.front()[0] == '{');
		}	
		else
		{
			if (!this->chunks.front().empty())
				return (false);
			this->chunks.pop_front();
			std::list<std::string>::iterator it = this->chunks.begin();
			if (this->chunks.front() == "=")
				if (++it == this->chunks.end())
					return (false);
			if ((*it).find("{") != std::string::npos || (++it != this->chunks.end() && (*it)[0] == '{'))
				return (true);
		}
	}
	return (false);
}

bool parser::is_directive(std::string directive)
{
	if (directive.compare(0, directive.length(), this->chunks.front()))
		return (false);
	this->chunks.front() = this->chunks.front().substr(directive.length(), this->chunks.front().length() - directive.length());
	if (this->chunks.front().empty())
		this->chunks.pop_front();
	return (true);
}
