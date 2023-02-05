#include "server.hpp"

namespace ft
{
	server::server() : base_dir_ext(), names(), listens() {}

	server::~server() {}

	server::server(const server &other) : base_dir_ext(other), names(other.names), listens(other.listens) {}

	server::server(const base_dir &other) : base_dir_ext(other), names(), listens() {}

	server &server::operator=(const server &other)
	{
		base_dir_ext::operator=(other);
		this->names = other.names;
		this->listens = other.listens;
		return (*this);
	}

	const string_set &server::get_names() const
	{
		return (this->names);
	}

	const address_set &server::get_listens() const
	{
		return (this->listens);
	}

	void server::add_name(const std::string &name)
	{
		this->names.insert(name);
	}

	void server::add_listen(const std::string &host, unsigned int port = 80)
	{
		this->listens.insert(std::pair<std::string, unsigned int>(host, port));
	}
}