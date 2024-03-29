#include "location.hpp"

namespace ft
{
	location::location() : base_dir_ext(), _route(), _modifier(false), _methods(), _flush_methods(false) {}

	location::~location() {}

	location::location(const location &other) : base_dir_ext(other), _route(other._route), _modifier(other._modifier), _methods(other._methods), _flush_methods(false) {}

	location::location(const base_dir &other, const std::string &route) : base_dir_ext(other), _route(route), _modifier(false), _methods(), _flush_methods(false)
	{
		if (dynamic_cast<const location*>(&other))
		{
			_route = static_cast<const location&>(other)._route;
			_modifier = static_cast<const location&>(other)._modifier;
			_methods = static_cast<const location&>(other)._methods;
		}
	}

	location &location::operator=(const location &other)
	{
		base_dir_ext::operator=(other);
		_route = other._route;
		_modifier = other._modifier;
		_methods = other._methods;
		_flush_methods = other._flush_methods;
		return (*this);
	}

	const std::string &location::get_route() const
	{
		return (_route);
	}

	bool location::has_modifier() const
	{
		return (_modifier);
	}

	bool location::is_allowed_method(const std::string &method) const
	{
		if (_methods.empty())
			return (true);
		if (_methods.find(method) != _methods.end())
			return (true);
		return (false);
	}

	void location::set_route(const std::string &route, location *parent)
	{
		if (parent)
			if (route.compare(0, parent->get_route().length(), parent->get_route()))
				throw std::invalid_argument("location \"" + route + "\" is outside location \"" + parent->get_route() + "\"");
		_route = (ends_with(route, "/") ? route.substr(0, route.size() - 1) : route);
	}

	void location::set_modifier(bool modifier)
	{
		_modifier = modifier;
	}

	void location::add_method(const std::string &method) // what if there are 2 GET _methods
	{
		if (_methods.find(method) == _methods.end())
		{
			_methods.insert(method);
			return ;
		}
		throw std::invalid_argument("Method duplicated.");
	}

	void location::flush_methods()
	{
		if (!_flush_methods)
		{
			_methods.clear();
			_flush_methods = true;
		}
	}

	bool location::operator<(const location &rhs) const
	{
		if (rhs.has_modifier() == _modifier)
			return (rhs.get_route() != _route);
		return (true);
	}

	bool location::operator==(const location &rhs) const
	{
		return (rhs.has_modifier() == has_modifier() && rhs.get_route() == _route);
	}
}
