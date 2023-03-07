#include "webserv.hpp"

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

	void webserv::set_http(base_dir *protocol)
	{
		this->protocol = static_cast<http*>(protocol);
	}

	void webserv::verify_http()
	{
		if (!this->protocol)
		{
			std::cout << "http = null" << std::endl;
			return ;
		}
		// typing http
		print_base_dir(this->protocol);

		// typing servers
		server_vector &servers = const_cast<server_vector &>(this->protocol->get_servers());
		for (size_t i = 0; i < servers.size(); i++)
		{
			std::cout << " |S| inspecting server " << i << ":" << std::endl;
			for (string_set::iterator it = servers[i].get_names().begin();
				it != servers[i].get_names().end();
				it++)
				std::cout << "name: " << *it << std::endl;
			for (address_set::iterator it = servers[i].get_listens().begin();
				it != servers[i].get_listens().end();
				it++)
				std::cout << "listens on { " << it->first << " : " << it->second << " }" << std::endl;
			print_base_dir_ext(&servers[i]);
		}
	}

	void webserv::print_base_dir_ext(base_dir_ext *ptr)
	{
		print_base_dir(ptr);
		std::cout << "printing locations for the given server/location:" << std::endl;
		for (location_set::iterator it = const_cast<location_set&>(ptr->get_locations()).begin();
			it != ptr->get_locations().end();
			it++)
		{
			std::cout << " |L| inspecting location \"" << it->get_route() << "\":" << std::endl;
			for (string_map::const_iterator mit = it->get_cgi_map().begin(); mit != it->get_cgi_map().end(); mit++)
				std::cout << "cgi: { " << mit->first << " : " << mit->second << " }" << std::endl;
			for (string_set::iterator mit = it->get_methods().begin(); mit != it->get_methods().end(); mit++)
				std::cout << "method disallowed: " << *mit << std::endl;
			print_base_dir_ext(const_cast<location*>(&(*it)));
			std::cout << "is = specified? " << (it->has_modifier() ? "yes" : "no") << std::endl;
		}
		for (string_mmap::const_iterator it = ptr->get_redirects().begin();
			it != ptr->get_redirects().end();
			it++)
		{
			std::cout << "redirects " << it->first << " to " << it->second << std::endl;
		}
	}

	void webserv::print_base_dir(base_dir *ptr)
	{
		std::cout << "root: " << ptr->get_root() << std::endl;
		std::cout << "autoindex: " << ptr->get_autoindex() << std::endl;
		for (error_map::iterator it = ptr->get_error_map().begin();
			it != ptr->get_error_map().end();
			it++)
		{
			std::cout << "error_page: { " << it->first << " : " << it->second << " }" << std::endl;
		}
		std::cout << "client_max_body_size: " << ptr->get_client_max_body_size() << std::endl;
		for (size_t i = 0; i < ptr->get_indexes().size(); i++)
		{
			std::cout << "index: " << ptr->get_indexes()[i] << std::endl;
		}
	}

	int webserv::error(const std::string &message) const
	{
		std::cerr << "[webserv]: Error: " << message << std::endl;
		return (EXIT_FAILURE);
	}
}
