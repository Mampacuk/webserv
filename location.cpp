#include "location.hpp"

location::location() : base_dir_ext(), cgi(), methods(), rout(), modifier(false) {}

location::~location() {}

location::location(const location &other) : base_dir_ext(other), cgi(other.cgi), methods(other.methods), rout(other.rout), modifier(other.modifier) {}

location::location(const base_dir &other) : base_dir_ext(other), cgi(), methods(), rout(), modifier(false) {}

location &location::operator=(const location &other)
{
	base_dir_ext::operator=(other);
	this->cgi = other.cgi;
	this->methods = other.methods;
	this->rout = other.rout;
	this->modifier = other.modifier;
}

const std::string location::get_cgi(const std::string extension) const
{
	std::map<std::string, std::string>::const_iterator it;

	it = cgi.find(extension);
	if (it != cgi.end())
		return (it->second);
	return ("");
}

const std::string &location::get_rout() const
{
	return (this->rout);
}

bool location::method_allowed(const std::string &method) const
{
	if (methods.find(method) != methods.end())
		return (true);
	return (false);
}

bool location::has_modifier() const
{
	return (this->modifier);
}

void location::add_method(const std::string &method) // what if there are 2 GET methods
{
	if (method == "GET" || method == "POST" || method == "DELETE")
	{
		if (methods.find(method) != methods.end())
		{
			methods.insert(method);
			return ;
		}
	}
	throw std::invalid_argument("Parsing error!");
}

void location::add_cgi(const std::string &extension, const std::string &path)
{
	this->cgi.insert(extension, path);
}
