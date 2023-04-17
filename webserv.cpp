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

	int webserv::log(const std::string &message, const char *color) const
	{
		std::cout << "[webserv]: " << color << message << RESET << std::endl;
		return (EXIT_SUCCESS);
	}

	int webserv::receive_request(request &request)
	{
		char buffer[BUFSIZ] = {0}; // pass BUFSIZ - 1 so it's null-terminated
		int  bytes_read = recv(request.get_socket(), buffer, BUFSIZ - 1, 0);
		if (bytes_read <= 0)
		{
			if (!bytes_read)
				log("Connection closed by the client.", YELLOW);
			else
				error("recv() failed: closing connection.");
			close(request.get_socket()); // move into ~request()???
			return (EXIT_FAILURE);
		}
		try
		{
			if (!(request += buffer))	
				return (EXIT_SUCCESS);	// request wasn't fully received
			request.parse();
			generate_response(request);
		}
		catch (const http::protocol_error &e)
		{
			// generate_response(request, e);
		}
		return (EXIT_SUCCESS);
	}

	int webserv::generate_response(request&, int)
	{
		// Anahit's code
		// if (error) then...
		return (EXIT_SUCCESS);
	}

	int webserv::send_response(response&)
	{
		// Anahit's code
		return (EXIT_SUCCESS);
	}

	void webserv::start_service()
	{
		fd_set					master_set;
		fd_set					reading_set;
		fd_set					writing_set;
		int						desc_ready = 0;
		int_set					sockets = this->_protocol->initialize_master(master_set);
		request_list			requests;
		response_list			responses;
		int						max_sd = *(--sockets.end());
		const char				bars[] = {'\\', '|', '/', '-'};
		const int				nbars = sizeof(bars) / sizeof(bars[0]);
		int						bar_id = 0;
		struct timeval			timeout = {TIMEOUT_SEC, TIMEOUT_MICROSEC};

		while (true)
		{
			while (desc_ready == 0)
			{
				// copy the master set into the reading set so that select() doesn't modify it
				std::memcpy(&reading_set, &master_set, sizeof(master_set));
				// initialize the writing set
				FD_ZERO(&writing_set);
				for (response_list::iterator it = responses.begin(); it != responses.end(); it++)
					FD_SET(*it, &writing_set);
				std::cout << EL << bars[(bar_id = (bar_id >= nbars) ? 0 : bar_id + 1)] << std::flush;
				desc_ready = select(max_sd + 1, &reading_set, &writing_set, NULL, &timeout);
				if (desc_ready == -1)
				{
					error("select() call failed: " + std::string(strerror(errno)));
					desc_ready = 0;
				}
			}

			// accept() reading_set block
			for (int_set::iterator it = sockets.begin(); it != sockets.end(); it++)
			{
				if (FD_ISSET(*it, &reading_set))
				{
					int new_sd = accept(*it, NULL, NULL);
					if (new_sd == -1)
						error("Couldn't create a socket for accepted connection: " + std::string(strerror(errno)));
					else
					{
						if (fcntl(new_sd, F_SETFL, O_NONBLOCK) != -1)
						{
							requests.push_back(new_sd);
							FD_SET(new_sd, &master_set);
							max_sd = new_sd > max_sd ? new_sd : max_sd;
						}
						else
							error("Couldn't mark accepted connection non-blocking.");
					}
					break ;
				}
			}

			// recv() reading_set block 
			for (request_list::iterator it = requests.begin(); it != requests.end(); it++)
			{
				if (FD_ISSET(*it, &reading_set))
				{
					if (receive_request(*it) == EXIT_FAILURE)
					{
						FD_CLR(*it, &reading_set);
						FD_CLR(*it, &master_set);
						requests.erase(it);
					}
					// else must push back a new ready request to response_list
					break ;
				}
			}

			// send() writing_set block
			for (response_list::iterator it = responses.begin(); it != responses.end(); it++)
			{
				if (FD_ISSET(*it, &writing_set))
				{
					// call send_response(*it),
					// do other stuff...
					break ;
				}
			}
		}
	}
}
