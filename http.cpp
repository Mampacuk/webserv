#include "http.hpp"

http::http(): base_dir(), servers() {}

http::~http() {}

http::http(const http &other): base_dir(other), servers(other.servers) {}

http &http::operator=(const http &other)
{
	base_dir::operator=(other);
	this->servers = other.servers;
}

const std::vector<server> &http::get_servers() const
{
	return (this->servers);
}

void http::add_server(server server)
{
	this->servers.push_back(server);
}
