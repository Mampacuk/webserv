#include "http.hpp"

namespace ft
{
	http::http(): base_dir(), _servers() {}

	http::~http() {}

	http::http(const http &other): base_dir(other), _servers(other._servers) {}

	http &http::operator=(const http &other)
	{
		base_dir::operator=(other);
		this->_servers = other._servers;
		return (*this);
	}

	const server_vector &http::get_servers() const
	{
		return (this->_servers);
	}

	void http::add_server(server server)
	{
		this->_servers.push_back(server);
	}

	void http::close_server_sockets()
	{
		for (size_t i = 0; this->_servers.size(); i++)
			this->_servers[i].close_sockets();
	}

	// adds all sockets from all servers to `master_set`
	request_set	http::initialize_master(fd_set &master_set) const
	{
		request_set requests;

		FD_ZERO(&master_set);
		for (server_vector::const_iterator it = get_servers().begin(); it != get_servers().end(); it++)
			for (int_vector::const_iterator sock = it->get_sockets().begin(); sock != it->get_sockets().end(); sock++)
			{
				requests.insert(request(*sock));
				FD_SET(*sock, &master_set);
			}
		return (requests);
	}
}
