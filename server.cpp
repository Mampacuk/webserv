#include "server.hpp"
#include "webserv.hpp"

namespace ft
{
	server::server() : base_dir_ext(), _names(), _sockets() {}

	server::~server()
	{
		close_sockets();
	}

	server::server(const server &other) : base_dir_ext(other), _names(other._names), _sockets(other._sockets) {}

	server::server(const base_dir &other) : base_dir_ext(other), _names(), _sockets() {}

	server &server::operator=(const server &other)
	{
		base_dir_ext::operator=(other);
		this->_names = other._names;
		this->_sockets = other._sockets;
		return (*this);
	}

	const string_vector &server::get_names() const
	{
		return (this->_names);
	}

	const int_vector &server::get_sockets() const
	{
		return (this->_sockets);
	}

	void server::add_name(const std::string &name)
	{
		if (std::find(this->_names.begin(), this->_names.end(), name) == this->_names.end())
			this->_names.push_back(name);
	}

	void server::close_sockets()
	{
		for (size_t i = 0; i < this->_sockets.size(); i++)
			close(this->_sockets[i]);
		this->_sockets.clear();
	}

	void server::add_socket(const std::string &host, const std::string &port)
	{
		int	status;
		int	socket_fd;
		struct addrinfo hints;
		struct addrinfo *result, *rit;

		std::memset(&hints, 0, sizeof(struct addrinfo)); // clear hints structure
		hints.ai_family = AF_INET;						 // what family to search?
		hints.ai_socktype = SOCK_STREAM;				 // what type of _sockets?
		hints.ai_flags = AI_ADDRCONFIG;					 // only address families configured on the system
		if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result)) != 0)
			throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(status)));
		for (rit = result; rit != NULL; rit = rit->ai_next)
		{
			if ((socket_fd = socket(rit->ai_family, rit->ai_socktype, rit->ai_protocol)) == -1)
				continue ;	// not a critical error; exception is thrown when eventually `_sockets` is empty
			int on = 1;
			if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) == -1
				|| setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on) == -1)
				|| setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on) == -1))
			{
				close(socket_fd);
				throw std::runtime_error("Couldn't set socket options.");
			}
			if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
			{
				close(socket_fd);
				throw std::runtime_error("Couldn't set the flags of a socket.");
			}
			if (bind(socket_fd, rit->ai_addr, rit->ai_addrlen) == -1)
			{
				close(socket_fd);
				webserver.error("bind() to " + host + ":" + port + " failed (" + strerror(errno) + ")");
				continue ;	// not a critical error; exception is thrown when eventually `_sockets` is empty
			}
			if (listen(socket_fd, BACKLOG) == -1)
			{
				close(socket_fd);
				throw std::runtime_error("Failed listening on " + host + ":" + port + ".");
			}
			this->_sockets.push_back(socket_fd);
		}
		freeaddrinfo(result);
		if (this->_sockets.empty())
			throw std::runtime_error("Socket creation and binding failed for a server block.");
	}
}
