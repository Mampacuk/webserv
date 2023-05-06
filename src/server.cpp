#include "server.hpp"
#include "webserv.hpp"
#include "socket.hpp"

namespace ft
{
	server::server() : base_dir_ext(), _names() {}

	server::~server() {}

	server::server(const server &other) : base_dir_ext(other), _names(other._names) {}

	server::server(const base_dir &other) : base_dir_ext(other), _names() {}

	server &server::operator=(const server &other)
	{
		base_dir_ext::operator=(other);
		this->_names = other._names;
		return (*this);
	}

	const string_vector &server::get_names() const
	{
		return (this->_names);
	}

	void server::add_name(const std::string &name)
	{
		if (std::find(this->_names.begin(), this->_names.end(), name) == this->_names.end())
			this->_names.push_back(name);
	}
}
