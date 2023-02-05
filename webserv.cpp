#include "webserv.hpp"
#include <iostream>

namespace ft
{
	webserv::webserv(): protocol() {}

	webserv::~webserv()
	{
		delete this->protocol;
	}

	webserv::webserv(const webserv &other): protocol(new http(*other.protocol)) {}

	webserv &webserv::operator=(const webserv &other)
	{
		*this->protocol = *other.protocol;
		return (*this);
	}

	http &webserv::get_http()
	{
		return (*this->protocol);
	}

	void webserv::set_http(http *protocol)
	{
		this->protocol = protocol;
	}

	void webserv::verify_http()
	{
		if (!this->protocol)
		{
			std::cout << "http = null" << std::endl;
			return ;
		}
		print_base_dir(this->protocol);
		// const server_vector &servers = this->protocol->get_servers();
		// for (size_t i = 0; i < servers.size(); i++)
		// {
		// 	std::cout << "inspecting server " << i << ":" << std::endl;
		// 	for (string_set::iterator it = servers[i].get_names().begin();
		// 		it != servers[i].get_names().end();
		// 		it++)
		// 		std::cout << "name: " << *it << std::endl;
		// 	for (address_set::iterator it = servers[i].get_listens().begin();
		// 		it != servers[i].get_listens().end();
		// 		it++)
		// 		std::cout << "listens on " << it->first << ":" << it->second << std::endl;
		// 	std::cout << std::endl;
		// }
	}

	void webserv::print_base_dir(base_dir *ptr)
	{
		std::cout << "root: " << ptr->get_root() << std::endl;
		std::cout << "autoindex: " << ptr->get_autoindex() << std::endl;
		for (error_map::iterator it = ptr->get_error_map().begin();
			it != ptr->get_error_map().end();
			it++)
			std::cout << "error_page: " << it->first << ":" << it->second << std::endl;
		std::cout << "client_max_body_size: " << ptr->get_client_max_body_size() << std::endl;
		for (size_t i = 0; i < ptr->get_indexes().size(); i++)
			std::cout << "index: " << ptr->get_indexes()[i] << std::endl;
	}
}
