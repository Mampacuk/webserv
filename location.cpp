#include "location.hpp"
#include <iostream> //remove later

namespace ft
{
	location::location() : base_dir_ext(), cgi(), methods(), route(), modifier(false) {}

	location::~location() {}

	location::location(const location &other) : base_dir_ext(other), cgi(other.cgi), methods(other.methods), route(other.route), modifier(other.modifier)
	{
		std::cout << "COPY-CTOR ENTERED" << std::endl;
	}

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
		string_map::const_iterator it;

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
		if (this->methods.find(method) != this->methods.end())
			return (true);
		return (false);
	}

	bool location::has_modifier() const
	{
		return (this->modifier);
	}

	void location::add_cgi(const std::string &extension, const std::string &path)
	{
		this->cgi.insert(string_pair(extension, path));
	}

	void location::set_route(const std::string &route)
	{
		this->route = route;
	}

	void location::add_method(const std::string &method) // what if there are 2 GET methods
	{
		if (method == "GET" || method == "POST" || method == "DELETE")
		{
			if (this->methods.find(method) == this->methods.end())
			{
				this->methods.insert(method);
				return ;
			}
		}
		throw std::invalid_argument("Method unknown or duplicated.");
	}

	void location::set_modifier(bool modifier)
	{
		this->modifier = modifier;
	}

	bool location::operator<(const location &rhs) const
	{
		if (rhs.has_modifier() == has_modifier())
			return (rhs.get_route() != get_route());
		return (true);
	}

	void location::flush_cgi()
	{
		this->cgi.clear();
	}

	void location::flush_methods()
	{
		this->methods.clear();
	}
}
