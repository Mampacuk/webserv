#include "server.hpp"
#include "webserv.hpp"

namespace ft
{
	server::server() : base_dir_ext(), names(), sockets() {}

	server::~server() {}

	server::server(const server &other) : base_dir_ext(other), names(other.names), sockets(other.sockets) {}

	server::server(const base_dir &other) : base_dir_ext(other), names(), sockets() {}

	server &server::operator=(const server &other)
	{
		base_dir_ext::operator=(other);
		this->names = other.names;
		this->sockets = other.sockets;
		return (*this);
	}

	const string_vector &server::get_names() const
	{
		return (this->names);
	}

	const int_vector &server::get_sockets() const
	{
		return (this->sockets);
	}

	void server::add_name(const std::string &name)
	{
		if (std::find(this->names.begin(), this->names.end(), name) == this->names.end())
			this->names.push_back(name);
	}

	void server::add_socket(const std::string &host, const std::string &port)
	{
		int	status;
		int	socket_fd;
		struct addrinfo hints;
		struct addrinfo *result, *rit;

		std::memset(&hints, 0, sizeof(struct addrinfo)); // clear hints structure
		hints.ai_family = AF_INET;						 // what family to search?
		hints.ai_socktype = SOCK_STREAM;				 // what type of sockets?
		hints.ai_flags = AI_ADDRCONFIG;					 // only address families configured on the system
		if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result)) != 0)
			throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(status)));
		for (rit = result; rit != NULL; rit = rit->ai_next)
		{
			if ((socket_fd = socket(rit->ai_family, rit->ai_socktype, rit->ai_protocol)) == -1)
				continue ;
			if (bind(socket_fd, rit->ai_addr, rit->ai_addrlen) == -1)
			{
				close(socket_fd);
				webserver.error("bind() to " + host + ":" + port + " failed (" + strerror(errno) + ")");
				continue ;
			}
			this->sockets.push_back(socket_fd);
		}
		freeaddrinfo(result);
		if (this->sockets.empty())
			throw std::runtime_error("Socket creation and binding failed for a server block.");
	}
}
