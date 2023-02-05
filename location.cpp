#include "location.hpp"

namespace ft
{
	location::location() : base_dir_ext(), cgi(), methods(), route(), modifier(false) {}

	location::~location() {}

	location::location(const location &other) : base_dir_ext(other), cgi(other.cgi), methods(other.methods), route(other.route), modifier(other.modifier) {}

	location::location(const base_dir &other) : base_dir_ext(other), cgi(), methods(), route(), modifier(false) {}

	location &location::operator=(const location &other)
	{
		base_dir_ext::operator=(other);
		this->cgi = other.cgi;
		this->methods = other.methods;
		this->route = other.route;
		this->modifier = other.modifier;
		return (*this);
	}

	const std::string location::get_cgi(const std::string extension) const
	{
		std::map<std::string, std::string>::const_iterator it;

		it = cgi.find(extension);
		if (it != cgi.end())
			return (it->second);
		return ("");
	}

	const std::string &location::get_route() const
	{
		return (this->route);
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
		this->cgi.insert(std::pair<std::string, std::string>(extension, path));
	}


	bool location::operator<(const location &rhs) const
	{
		if (rhs.has_modifier() == has_modifier())
			return (rhs.get_route() != get_route());
		return (true);
	}
}
