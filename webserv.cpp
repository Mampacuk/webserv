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
		int  bytes_read = recv(socket, buffer, BUFSIZ - 1, 0);
		if (bytes_read <= 0)
		{
			if (!bytes_read)
				log("Connection closed by the client.", YELLOW);
			else
				error("recv() failed: closing connection.");
			close(socket);
			messages.erase(socket);
			return (EXIT_FAILURE);
		}
		messages[socket] += buffer;
		const size_t headers_end = messages[socket].find(CLRF CLRF);
		if (headers_end != std::string::npos) // headers fully accepted
		{
			request request(messages[socket]);
			if (request["Content-Length"].empty())
			{
				if (request["Transfer-Encoding"] == "chunked")
				{
					if (ends_with(messages[socket], "0" CLRF CLRF))
						return (generate_response(request));
					return (EXIT_SUCCESS);
				}
				else if (request["Transfer-Encoding"].empty())
					return (generate_response(request));
				else // else the message is ill-formed
				{
					error("Invalid request received: `Transfer-Encoding` error.");
					return (EXIT_SUCCESS);
				}
			}

			try
			{
				const size_t content_len = parser::strtoul(request["Content-Length"]);
				if (messages[socket].size() == content_len + headers_end + std::strlen(CLRF CLRF CLRF))
					return (generate_response(request));
			}
			catch (const std::exception &e)
			{
				error("Invalid request received: `Content-Length` error.");
			}
			// push_back socket to ready? becomes a part of writing fd?
		}
		return (EXIT_SUCCESS);
	}

	int webserv::generate_response(request &request)
	{
		// Anahit's code
	}

	void webserv::start_service()
	{
		fd_set					master_set;
		fd_set					reading_set;
		fd_set					writing_set;
		int						desc_ready = 0;
		request_set				requests = this->_protocol->initialize_master(master_set);
		int						max_sd = (--requests.end())->first;
		const char				bars[] = {'\\', '|', '/', '-'};
		const int				nbars = sizeof(bars) / sizeof(bars[0]);
		int						bar_id = 0;
		const struct timeval	timeout = {TIMEOUT_SEC, TIMEOUT_MICROSEC};

		while (true)
		{
			while (desc_ready == 0)
			{
				// copy the master set into the reading set so that select() doesn't modify it
				std::memcpy(&reading_set, &master_set, sizeof(master_set));
				FD_ZERO(&writing_set);
				// something with writing_set
				std::cout << EL << bars[(bar_id = (bar_id >= nbars) ? 0 : bar_id + 1)] << std::flush;
				desc_ready = select(max_sd + 1, &reading_set, &writing_set, NULL, &timeout);
				if (desc_ready == -1)
				{
					error("select() call failed: " + std::string(strerror(errno)));
					desc_ready = 0;
				}
			}
			// writing set block
			log("Received a connection.", GREEN);
			for (int i = 0; i <= max_sd && desc_ready > 0; i++)
			{
				if (FD_ISSET(i, &reading_set))
				{
					desc_ready--;
					if (requests.find(i) != requests.end()) // ACCEPT BLOCK
					{
						int new_sd = accept(i, NULL, NULL);
						if (new_sd == -1)
							error("Couldn't create a socket for accepted connection: " + std::string(strerror(errno)));
						else
						{
							if (fcntl(new_sd, F_SETFL, O_NONBLOCK) != -1)
							{
								requests.insert(new_sd);
								FD_SET(new_sd, &master_set);
								max_sd = new_sd > max_sd ? new_sd : max_sd;
							}
							else
								error("Couldn't mark accepted connection non-blocking.");
						}
					}
					else // RECV BLOCK
					{
						if (receive_request(i, requests) == EXIT_FAILURE)
						{
							FD_CLR(i, &reading_set);
							FD_CLR(i, &master_set);
						}
						//..// push_back socket to ready? becomes a part of writing fd?
					}
				}
			}
		}
	}
}
