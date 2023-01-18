#ifndef HTTP_HPP
# define HTTP_HPP

# include "base_dir.hpp"
# include "server.hpp"

# include <vector>

class http : public base_dir
{
	private:
		std::vector<server>	servers;
	public:
		http();
		~http();
		http(const http &other);
		http &operator=(const http &other);
		const std::vector<server> &get_servers() const;
		void add_server(server server);
};

#endif
