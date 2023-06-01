#include "webserv.hpp"

namespace ft
{
	webserv::webserv() : _protocol() {}

	webserv::~webserv()
	{
		delete _protocol;
	}

	webserv::webserv(const webserv &other) : _protocol(new http(*other._protocol)) {}

	webserv &webserv::operator=(const webserv &other)
	{
		*_protocol = *other._protocol;
		return (*this);
	}

	const http &webserv::get_http() const
	{
		return (*_protocol);
	}

	void webserv::set_http(base_dir *protocol)
	{
		_protocol = static_cast<http*>(protocol);
	}

	void webserv::serve() const
	{
		fd_set					master_set;
		fd_set					reading_set;
		fd_set					writing_set;
		request_list			requests;
		response_list			responses;
		int						desc_ready = 0;
		struct timeval			timeout = {TIMEOUT_SEC, TIMEOUT_MICROSEC};
		const server_socket_set	&sockets = _protocol->get_sockets();
		int						max_sd = *(--sockets.end());

		FD_ZERO(&master_set);
		for (server_socket_set::const_iterator sock = sockets.begin(); sock != sockets.end(); sock++)
			FD_SET(*sock, &master_set);
		while (true)
		{
			// select() block
			while (desc_ready == 0)
			{
				// copy the master set into the reading set so that select() doesn't modify it
				std::memcpy(&reading_set, &master_set, sizeof(master_set));
				// initialize the writing set
				FD_ZERO(&writing_set);
				for (response_list::iterator it = responses.begin(); it != responses.end(); it++)
					FD_SET(*it, &writing_set);
				desc_ready = select(max_sd + 1, &reading_set, &writing_set, NULL, &timeout);
				if (desc_ready == -1)
				{
					error("[SELECT] select() call failed: " + std::string(strerror(errno)));
					desc_ready = 0;
				}
			}
			// accept() reading_set block
			for (server_socket_set::iterator it = sockets.begin(); it != sockets.end(); it++)
			{
				if (FD_ISSET(*it, &reading_set))
				{
					client_socket new_sd(accept_connection(*it));
					if (new_sd == -1)
						error("[ACCEPT] Couldn't create a socket for accepted connection: " + std::string(strerror(errno)));
					else
					{
						requests.push_back(new_sd);
						FD_SET(new_sd, &master_set);
						max_sd = new_sd.get_fd() > max_sd ? new_sd.get_fd() : max_sd;
					}
					break ;
				}
			}
			// recv() reading_set block 
			for (request_list::iterator it = requests.begin(); it != requests.end(); it++)
			{
				if (FD_ISSET(*it, &reading_set))
				{
					if (receive_request(*it, responses) == EXIT_SUCCESS)
					{
						FD_CLR(*it, &reading_set);
						FD_CLR(*it, &master_set);
						requests.erase(it);
					}
					break ;
				}
			}
			// send() writing_set block
			for (response_list::iterator it = responses.begin(); it != responses.end(); it++)
			{
				if (FD_ISSET(*it, &writing_set))
				{
					if (send_response(*it) == EXIT_SUCCESS)
					{
						FD_CLR(*it, &writing_set);
						FD_CLR(*it, &master_set);
						close(*it);
						responses.erase(it);
					}
					break ;
				}
			}
			desc_ready = 0; // renew the select() loop
		}
	}

	int webserv::error(const std::string &error)
	{
		std::cerr << "[" CYAN BOLDED("webserv") RESET "]: " RED BOLDED("Error") LRED ": " << error << RESET << std::endl;
		return (EXIT_FAILURE);
	}

	int webserv::log(const std::string &msg, const char *color)
	{
		std::cout << "[" CYAN BOLDED("webserv") RESET "]: " << color << msg << RESET << std::endl;
		return (EXIT_SUCCESS);
	}

	client_socket webserv::accept_connection(const server_socket &socket) const
	{
		int client_fd;
		std::string host;
		std::string port;
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);

		client_fd = accept(socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
		if (client_fd == -1 || fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
			return (client_socket(client_fd, "", "", socket));
		host = ft::inet_ntoa(client_addr.sin_addr);
		port = ft::to_string(ntohs(client_addr.sin_port));
		log(BOLDED("[ACCEPT]") " Connected client " + host + ":" + port + " to address " + socket.get_host() + ":" + socket.get_port() + ".", GREEN);
		return (client_socket(client_fd, host, port, socket));
	}

	int webserv::receive_request(request &request, response_list &responses) const
	{
		char buffer[BUFSIZ]; // pass BUFSIZ - 1 so it's null-terminated
		int  bytes_read = recv(request, buffer, BUFSIZ - 1, 0);
		if (bytes_read <= 0)
		{
			if (!bytes_read)
				log(BOLDED("[RECV]") " Connection closed by the client.", YELLOW);
			else
				error(BOLDED("[RECV]") " recv() failed due to " + to_string(errno) + " " + strerror(errno));
			close(request);
			return (EXIT_SUCCESS);
		}
		buffer[bytes_read] = '\0';
		log(BOLDED("[RECV]") " Received " + to_string(bytes_read) + " bytes from " + to_string(request), GREEN);
		try
		{
			if (!(request.append_chunk(buffer, bytes_read)))	
				return (EXIT_FAILURE);	// request wasn't fully received
			request.parse();
			log(BOLDED("[RECV]") " Pushed " + to_string(request) + " to responses", GREEN);
			responses.push_back(response(request));
		}
		catch (const protocol_error &e)
		{
			responses.push_back(response(request, e));
		}
		return (EXIT_SUCCESS);
	}

	int webserv::send_response(response &response) const
	{
		char_vector_iterator_pair range = response.get_chunk();
		ssize_t bytes_written = send(response, &(*range.first), range.second - range.first, 0);
		if (bytes_written <= 0)
		{
			if (!bytes_written)
				log(BOLDED("[RECV]") " Connection closed by the client.", YELLOW);
			else
				error(BOLDED("[SEND]") " send() failed due to " + std::string(strerror(errno)));
			return (EXIT_SUCCESS);
		}
		if (!response.sent())
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}
}
