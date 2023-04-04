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
			for (string_vector::const_iterator it = servers[i].get_names().begin();
				it != servers[i].get_names().end();
				it++)
				std::cout << "name: " << *it << std::endl;
			for (int_vector::const_iterator it = servers[i].get_sockets().begin();
				it != servers[i].get_sockets().end();
				it++)
				std::cout << "listens on socket { " << *it << " }" << std::endl;
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
			std::cout << "cgi: { " << it->get_cgi().first << " : " << it->get_cgi().second << " }" << std::endl;
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

	void webserv::receive_header(int i)
	{
		std::string header;

		while (true)
		{
			char buffer[BUFSIZ];
			int  bytes_read = recv(i, buffer, sizeof(buffer), 0);
			if (bytes_read < 0)
			{
				// if ()
			}
		}
	}

	void webserv::start_service()
	{
		fd_set	master_set;
		fd_set	working_set;
		bool	end_server = false;
		int		desc_ready;
		int		new_sd = 0;
		int_set	sockets = this->protocol->initialize_master(master_set);
		int		max_sd = *(--sockets.end());
		bool	close_conn;

		while (end_server == false)
		{
			// copy the master set into the working set so that select() doesn't modify it
			std::memcpy(&working_set, &master_set, sizeof(master_set));
			desc_ready = select(max_sd + 1, &working_set, NULL, NULL, NULL);
			if (desc_ready == -1)
			{
				if (errno != EINTR && errno != EAGAIN) // allowed errors
				{
					this->protocol->close_server_sockets();
					throw std::runtime_error("select() function failed.");
				}
				continue ;
			}
			for (int i = 0; i <= max_sd && desc_ready > 0; i++)
			{
				if (FD_ISSET(i, &working_set))
				{
					desc_ready--;
					if (sockets.find(i) != sockets.end())
					{
						//One of the listening sockets is readable
						while (new_sd != -1)
						{
							new_sd = accept(i, NULL, NULL); // can get info about client
							if (new_sd == -1)
							{
								if (errno != EWOULDBLOCK)
								{
									error("accept() call failed on socket");
									FD_CLR(i, &master_set);
								}
								break ;
							}
							//New incoming connection is new_sd
							FD_SET(new_sd, &master_set);
							max_sd = new_sd > max_sd ? new_sd : max_sd;
						}
					}
					else
					{
						close_conn = false;
						receive_header(i);
						//..
					}
				}
			}
		}
	}
}
