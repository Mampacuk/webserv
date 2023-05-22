#include "webserv.hpp"

namespace ft
{
	webserv::webserv(): _protocol(), _environment() {}

	webserv::~webserv()
	{
		delete this->_protocol;
	}

	webserv::webserv(const webserv &other) : _protocol(new http(*other._protocol)), _environment(other._environment) {}

	webserv &webserv::operator=(const webserv &other)
	{
		*this->_protocol = *other._protocol;
		return (*this);
	}

	const http &webserv::get_http() const
	{
		return (*this->_protocol);
	}

	char **webserv::get_environ() const
	{
		return (this->_environment);
	}

	void webserv::set_http(base_dir *protocol)
	{
		this->_protocol = static_cast<http*>(protocol);
	}

	void webserv::set_environment(char **environment)
	{
		this->_environment = environment;
	}

	int webserv::error(const std::string &error) const
	{
		std::cerr << "[webserv]: " RED "Error" RESET ": " << error << std::endl;
		return (EXIT_FAILURE);
	}

	int webserv::log(const std::string &msg, const char *color) const
	{
		std::cout << "[webserv]: " << color << msg << RESET << std::endl;
		return (EXIT_SUCCESS);
	}

	int webserv::label_log(const std::string &msg, const std::string &label, const char *msg_color, const char *label_color) const
	{
		std::cout << "[webserv]: [" << label_color << label << RESET "] " << msg_color << msg << RESET << std::endl;
		return (EXIT_SUCCESS);
	}

	client_socket webserv::accept_connection(const server_socket &socket)
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
		port = ft::to_string(ntohs(client_addr.sin_port)); // works incorrectly
		label_log("Successfully accepted connection from " + host + ":" + port, BOLDED("ACCEPT"), GREEN, LGREEN);
		return (client_socket(client_fd, host, port, socket));
	}

	int webserv::receive_request(request &request, response_list &responses)
	{
		char buffer[BUFSIZ]; // pass BUFSIZ - 1 so it's null-terminated
		int  bytes_read = recv(request, buffer, BUFSIZ - 1, 0);
		if (bytes_read <= 0)
		{
			if (!bytes_read)
				label_log("Connection closed by the client.", BOLDED("RECV"), YELLOW);
			else
				error("[RECEIVE] recv() failed due to " + to_string(errno) + " " + strerror(errno));
			close(request);
			return (EXIT_SUCCESS);
		}
		buffer[bytes_read] = '\0';
		label_log("Received " + to_string(bytes_read) + " bytes from " + to_string(request), BOLDED("RECV"), GREEN, LGREEN);
		try
		{
			if (!(request += buffer))	
				return (EXIT_FAILURE);	// request wasn't fully received
			request.parse();
			label_log("Pushed " + to_string(request) + " to responses", BOLDED("RECV"), GREEN, LGREEN);
			responses.push_back(response(request));
		}
		catch (const protocol_error &e)
		{
			responses.push_back(response(request, e));
		}
		return (EXIT_SUCCESS);
	}

	int webserv::send_response(response &response)
	{
		std::string chunk = response.get_chunk();
		ssize_t bytes_written = send(response, chunk.c_str(), chunk.size(), 0);
		if (bytes_written == -1)
		{
			error("[SEND] send() failed.");
			return (EXIT_SUCCESS);
		}
		// label_log("Sent successfully " + to_string(bytes_written) + " bytes from " + to_string(response), BOLDED("SEND"), GREEN, GREEN);
		if (!response.sent())
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}

	void webserv::serve()
	{
		fd_set					master_set;
		fd_set					reading_set;
		fd_set					writing_set;
		request_list			requests;
		response_list			responses;
		int						desc_ready = 0;
		// int						bar_id = 0;
		// const char				bars[] = {'\\', '|', '/', '-'};
		// const int				nbars = sizeof(bars) / sizeof(bars[0]);
		struct timeval			timeout = {TIMEOUT_SEC, TIMEOUT_MICROSEC};
		const server_socket_set	&sockets = this->_protocol->get_sockets();
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
				// std::cout << "\rWaiting for a connection... " << bars[(bar_id = (bar_id >= nbars - 1) ? 0 : bar_id + 1)] << std::flush;
				desc_ready = select(max_sd + 1, &reading_set, &writing_set, NULL, &timeout);
				if (desc_ready > 0)
				{
					std::cout << std::endl;
					// label_log("select() returned " + to_string(desc_ready) + " descriptors", BOLDED("SELECT"), YELLOW);
				}
				if (desc_ready == -1)
				{
					error("[SELECT] select() call failed: " + std::string(strerror(errno)));
					desc_ready = 0;
				}
			}
			// log("Responses: " + to_string(responses.size()) + ", Requests: " + to_string(requests.size()), MAGENTA);
			bool entered = false;
			// accept() reading_set block
			for (server_socket_set::iterator it = sockets.begin(); it != sockets.end(); it++)
			{
				entered = true;
				// log("Determining if server socket " + to_string(*it) + " is set...", RED);
				if (FD_ISSET(*it, &reading_set))
				{
					client_socket new_sd(accept_connection(*it));
					if (new_sd == -1)
					{
						error("[ACCEPT] Couldn't create a socket for accepted connection: " + std::string(strerror(errno)));
					}
					else
					{
						// label_log("Successfully created " + to_string(new_sd) + " from " + to_string(*it), BOLDED("ACCEPT"), GREEN, LGREEN);
						requests.push_back(new_sd);
						FD_SET(new_sd, &master_set);
						FD_SET(new_sd, &reading_set);
						max_sd = new_sd.get_fd() > max_sd ? new_sd.get_fd() : max_sd;
					}
					break ;
				}
				// else log("Server socket " + to_string(*it) + " wasn't set.", LRED);
			}
			// if (!entered) log("Server sockets are empty.", RED);
			entered = false;
			// log("Responses: " + to_string(responses.size()) + ", Requests: " + to_string(requests.size()), MAGENTA);

			// recv() reading_set block 
			for (request_list::iterator it = requests.begin(); it != requests.end(); it++)
			{
				entered = true;
				// log("Determining if request " + to_string(*it) + " is set...", RED);
				if (FD_ISSET(*it, &reading_set))
				{
					label_log("About to receive from " + to_string(*it), BOLDED("RECV"), GREEN, LGREEN);
					if (receive_request(*it, responses) == EXIT_SUCCESS)
					{
						FD_CLR(*it, &reading_set);
						FD_CLR(*it, &master_set);
						requests.erase(it);
					}
					break ;
				}
				// else log("Request " + to_string(*it) + " wasn't set.", LRED);
			}
			// if (!entered) log("Requests are empty.", RED);
			entered = false;
			// log("Responses: " + to_string(responses.size()) + ", Requests: " + to_string(requests.size()), MAGENTA);

			// send() writing_set block
			for (response_list::iterator it = responses.begin(); it != responses.end(); it++)
			{
				entered = true;
				// log("Determining if response " + to_string(*it) + " is set...", RED);
				if (FD_ISSET(*it, &writing_set))
				{
					// label_log("About to send from " + to_string(*it), BOLDED("SEND"), GREEN, YELLOW);
					if (send_response(*it) == EXIT_SUCCESS)
					{
						// label_log("Ended connection of " + to_string(*it), BOLDED("SEND"), GREEN, YELLOW);
						FD_CLR(*it, &writing_set);
						FD_CLR(*it, &master_set);
						close(*it);
						responses.erase(it);
					}
					break ;
				}
				// else log("Response " + to_string(*it) + " wasn't set.", LRED);
			}
			// if (!entered) log("Responses are empty.", RED);
			// log("Responses: " + to_string(responses.size()) + ", Requests: " + to_string(requests.size()), MAGENTA);
			desc_ready = 0;
		}
	}
}
