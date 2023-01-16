#ifndef SERVER_HPP
# define SERVER_HPP

# include "base_dir_ext.hpp"
# include "location.hpp"

class server : public base_dir_ext
{
	private:
		std::set<std::string> names;
		std::set<std::pair<std::string, unsigned int>> listens;
	public:
		server();
		~server();
		server(const server &other);
		server(const base_dir &other);
		server &operator=(const server &other);
		const std::set<std::string> &get_names() const;
		const std::set<std::pair<std::string, unsigned int>> &get_listens();
};

#endif
