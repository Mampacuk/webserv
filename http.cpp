#include "http.hpp"

namespace ft
{
	http::http(): base_dir(), servers() {}

	http::~http() {}

	http::http(const http &other): base_dir(other), servers(other.servers) {}

	http &http::operator=(const http &other)
	{
		base_dir::operator=(other);
		this->servers = other.servers;
		return (*this);
	}

	const server_vector &http::get_servers() const
	{
		return (this->servers);
	}

	void http::add_server(server server)
	{
		this->servers.push_back(server);
	}

	void http::close_server_sockets()
	{
		for (size_t i = 0; this->servers.size(); i++)
			this->servers[i].close_sockets();
	}

	// adds all sockets from all servers to `master_set`; returns the max_fd among them.
	int_string_map	http::initialize_master(fd_set &master_set) const
	{
		int_string_map socket_messages;

		FD_ZERO(&master_set);
		for (server_vector::const_iterator it = get_servers().begin(); it != get_servers().end(); it++)
			for (int_vector::const_iterator sock = it->get_sockets().begin(); sock != it->get_sockets().end(); sock++)
			{
				socket_messages.insert(int_string(*sock, ""));
				FD_SET(*sock, &master_set);
			}
		return (socket_messages);
	}
}
