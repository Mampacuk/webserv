#include "location.hpp"

namespace ft
{
	location::location() : base_dir_ext(), _methods(), _route(), _modifier(false) {}

	location::~location() {}

	location::location(const location &other) : base_dir_ext(other), _methods(other._methods), _route(other._route), _modifier(other._modifier) {}

	location::location(const base_dir &other, const std::string &route) : base_dir_ext(other), _methods(), _route(route), _modifier(false)
	{
		if (dynamic_cast<const location*>(&other))
		{
			this->_methods = static_cast<const location&>(other)._methods;
			this->_route = static_cast<const location&>(other)._route;
			this->_modifier = static_cast<const location&>(other)._modifier;
		}
	}

	location &location::operator=(const location &other)
	{
		base_dir_ext::operator=(other);
		this->_methods = other._methods;
		this->_route = other._route;
		this->_modifier = other._modifier;
		return (*this);
	}

	const std::string &location::get_route() const
	{
		return (this->_route);
	}

	bool location::is_allowed_method(const std::string &method) const
	{
		if (_methods.empty())
			return (true);
		if (this->_methods.find(method) != this->_methods.end())
			return (true);
		return (false);
	}

	bool location::has_modifier() const
	{
		return (this->_modifier);
	}

	void location::set_route(const std::string &_route, location *parent)
	{
		if (parent)
			if (_route.compare(0, parent->get_route().length(), parent->get_route()))
				throw std::invalid_argument("location \"" + _route + "\" is outside location \"" + parent->get_route() + "\"");
		this->_route = _route;
	}

	void location::add_method(const std::string &method) // what if there are 2 GET _methods
	{
		if (method == "GET" || method == "POST" || method == "DELETE")
		{
			if (this->_methods.find(method) == this->_methods.end())
			{
				this->_methods.insert(method);
				return ;
			}
		}
		throw std::invalid_argument("Method unknown or duplicated.");
	}

	void location::set_modifier(bool _modifier)
	{
		this->_modifier = _modifier;
	}

	void location::flush_methods()
	{
		this->_methods.clear();
	}

	bool location::operator<(const location &rhs) const
	{
		if (rhs.has_modifier() == has_modifier())
			return (rhs.get_route() != get_route());
		return (true);
	}
}
