#include "location.hpp"
#include <iostream> //remove later

namespace ft
{
	location::location() : base_dir_ext(), cgi(), methods(), route(), modifier(false) {}

	location::~location() {}

	location::location(const location &other) : base_dir_ext(other), cgi(other.cgi), methods(other.methods), route(other.route), modifier(other.modifier)
	{
		// std::cout << "COPY-CTOR FOR LOCATION ENTERED" << std::endl;
	}

	// location::location(const server &other) : base_dir_ext(other), cgi(), methods(), route(), modifier(false)
	// {
	// 	std::cout << "COPY-CTOR FOR SERVER ENTERED" << std::endl;
	// }

	location::location(const base_dir &other) : base_dir_ext(other), cgi(), methods(), route(), modifier(false)
	{
			// std::cout << "received " << &other << " to copy from for " << this << std::endl;
		if (dynamic_cast<const location*>(&other))
		{
			// std::cout << "COPY-CTORED LOCATION MEMBERS" << std::endl;
			this->cgi = dynamic_cast<const location*>(&other)->cgi;
			this->methods = dynamic_cast<const location*>(&other)->methods;
		}
		
		// std::cout << "COPY-CTOR FOR BASE_DIR_EXT ENTERED" << std::endl;
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
		this->cgi.first.clear();
		this->cgi.second.clear();
	}

	void location::flush_methods()
	{
		this->methods.clear();
	}
}
