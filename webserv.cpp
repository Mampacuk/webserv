#include "webserv.hpp"

namespace ft
{
	webserv::webserv(): _protocol() {}

	webserv::~webserv()
	{
		delete this->_protocol;
	}

	webserv::webserv(const webserv &other): _protocol(new http(*other._protocol)) {}

	webserv &webserv::operator=(const webserv &other)
	{
		*this->_protocol = *other._protocol;
		return (*this);
	}

	http &webserv::get_http()
	{
		return (*this->_protocol);
	}

	void webserv::set_http(base_dir *protocol)
	{
		this->_protocol = static_cast<http*>(protocol);
	}

	int webserv::error(const std::string &error) const
	{
		std::cerr << "[webserv]: " << RED << "Error" << RESET << ": " << error << std::endl;
		return (EXIT_FAILURE);
	}

	int webserv::log(const std::string &message) const
	{
		std::cout << "[webserv]: " << message << std::endl;
		return (EXIT_SUCCESS);
	}

	// 
	int webserv::receive_request(int socket, int_string_map &requests)
	{
		char buffer[BUFSIZ] = {0};
		int  bytes_read = recv(socket, buffer, BUFSIZ - 1, 0);
		if (bytes_read <= 0)
		{
			if (!bytes_read)
				log("Connection closed by the client.");
			else
				log("recv() failed: closing connection.");
			close(socket);
			requests.erase(socket);
			return (EXIT_FAILURE);
		}
		requests[socket] += buffer;
		size_t i = requests[socket].find(CLRF CLRF);
		if (i != std::string::npos) // message fully accepted
		{
			request request(requests[socket]);


		}
		return (EXIT_SUCCESS);
	}

	void webserv::start_service()
	{
		fd_set			master_set; // this is reading_set; need writing_set also, passed to select
		fd_set			working_set;
		bool			end_server = false;
		int				desc_ready;
		int				new_sd = 0;
		int_string_map	requests = this->_protocol->initialize_master(master_set);
		int				max_sd = (--requests.end())->first;

		while (end_server == false)
		{
			// copy the master set into the working set so that select() doesn't modify it
			std::memcpy(&working_set, &master_set, sizeof(master_set));
			desc_ready = select(max_sd + 1, &working_set, NULL, NULL, NULL);
			if (desc_ready == -1)
			{
				if (errno != EINTR && errno != EAGAIN) // allowed errors
				{
					this->_protocol->close_server_sockets();
					throw std::runtime_error("select() function failed.");
				}
				continue ;
			}
			for (int i = 0; i <= max_sd && desc_ready > 0; i++)
			{
				if (FD_ISSET(i, &working_set))
				{
					desc_ready--;
					if (requests.find(i) != requests.end())
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
						if (receive_request(i, requests) == EXIT_FAILURE)
						{
							FD_CLR(i, &working_set);
							FD_CLR(i, &master_set);
						}
						//..
					}
				}
			}
		}
	}
}
