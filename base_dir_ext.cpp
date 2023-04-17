#include "base_dir_ext.hpp"

namespace ft
{
	base_dir_ext::base_dir_ext() : base_dir(), _redirects(), _locations() {}

	base_dir_ext::~base_dir_ext() {}

	base_dir_ext::base_dir_ext(const base_dir_ext &other) : base_dir(other), _redirects(), _locations(other._locations) {}

	base_dir_ext::base_dir_ext(const base_dir &other) : base_dir(other), _redirects(), _locations() {}

	base_dir_ext &base_dir_ext::operator=(const base_dir_ext &other)
	{
		base_dir::operator=(other);
		this->_locations = other._locations;
		return (*this);
	}

	const string_mmap &base_dir_ext::get_redirects() const
	{
		return (this->_redirects);
	}

	const location_set &base_dir_ext::get_locations() const
	{
		return (this->_locations);
	}

	void base_dir_ext::add_redirect(std::string expression, std::string uri)
	{
		this->_redirects.insert(std::pair<std::string, std::string>(expression, uri));
	}

	void base_dir_ext::add_location(location location)
	{
		if (this->_locations.find(location) != this->_locations.end())
			throw parser::parsing_error("Duplicate location.");
		this->_locations.insert(location);
	}
}
