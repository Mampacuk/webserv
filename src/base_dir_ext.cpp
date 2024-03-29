#include "base_dir_ext.hpp"

namespace ft
{
	base_dir_ext::base_dir_ext() : base_dir(), _locations(), _rewrites() {}

	base_dir_ext::~base_dir_ext() {}

	base_dir_ext::base_dir_ext(const base_dir_ext &other) : base_dir(other), _locations(other._locations), _rewrites(other._rewrites) {}

	base_dir_ext::base_dir_ext(const base_dir &other) : base_dir(other), _locations(), _rewrites() {}

	base_dir_ext &base_dir_ext::operator=(const base_dir_ext &other)
	{
		base_dir::operator=(other);
		_locations = other._locations;
		_rewrites = other._rewrites;
		return (*this);
	}

	const location_set &base_dir_ext::get_locations() const
	{
		return (_locations);
	}

	const string_map &base_dir_ext::get_rewrites() const
	{
		return (_rewrites);
	}

	void base_dir_ext::add_location(location location)
	{
		if (_locations.find(location) != _locations.end())
			throw parsing_error("Duplicate location.");
		_locations.insert(location);
	}

	void base_dir_ext::add_rewrite(std::string expression, std::string uri)
	{
		_rewrites.insert(string_pair(expression, uri));
	}

	void base_dir_ext::flush_rewrites()
	{
		_rewrites.clear();
	}
}
