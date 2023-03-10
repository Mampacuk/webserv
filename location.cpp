#include "location.hpp"

namespace ft
{
	location::location() : base_dir_ext(), cgi(), methods(), route(), modifier(false) {}

	location::~location() {}

	location::location(const location &other) : base_dir_ext(other), cgi(other.cgi), methods(other.methods), route(other.route), modifier(other.modifier) {}

	location::location(const base_dir &other) : base_dir_ext(other), cgi(), methods(), route(), modifier(false)
	{
		if (dynamic_cast<const location*>(&other))
		{
			this->cgi = static_cast<const location&>(other).cgi;
			this->methods = static_cast<const location&>(other).methods;
			this->route = static_cast<const location&>(other).route;
			this->modifier = static_cast<const location&>(other).modifier;
		}
	}

	location &location::operator=(const location &other)
	{
		base_dir_ext::operator=(other);
		this->cgi = other.cgi;
		this->methods = other.methods;
		this->route = other.route;
		this->modifier = other.modifier;
		return (*this);
	}

	const string_pair &location::get_cgi() const
	{
		return (this->cgi);
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
		this->cgi.first = extension;
		this->cgi.second = path;
	}

	void location::set_route(const std::string &route, location *parent)
	{
		if (parent)
			if (route.compare(0, parent->get_route().length(), parent->get_route()))
				throw std::invalid_argument("location \"" + route + "\" is outside location \"" + parent->get_route() + "\"");
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
		this->cgi.first.clear();
		this->cgi.second.clear();
	}

	void location::flush_methods()
	{
		this->methods.clear();
	}
}
