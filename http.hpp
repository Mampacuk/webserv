#ifndef HTTP_HPP
# define HTTP_HPP

# include "base_dir.hpp"
# include "server.hpp"

# include <vector>

namespace ft
{
	typedef std::vector<server> server_vector;

	class http : public base_dir
	{
		private:
			server_vector	servers;
		public:
			http();
			~http();
			http(const http &other);
			http &operator=(const http &other);
			const server_vector &get_servers() const;
			void add_server(server server);
	};
}

#endif
