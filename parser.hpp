#ifndef PARSER_HPP
# define PARSER_HPP

# include "base_dir.hpp"
# include <stack>
# include <fstream>
# include <sstream>
# include <list>

class parser
{
	public:
		typedef std::map<std::string, bool (parser::*)(base_dir*)> directive_map;
		typedef std::map<std::string, base_dir *(parser::*)(base_dir*)> context_map;
		typedef std::pair<std::string, bool (parser::*)(base_dir*)> directive;
		typedef std::pair<std::string, base_dir *(parser::*)(base_dir*)> context;

		directive_map directives;
		context_map contexts;
		std::stack<char> braces;
		std::ifstream config;
		std::list<std::string> chunks;
	public:
		parser();
		~parser();
		parser(const parser &other);
		parser &operator=(const parser &other);
		base_dir *parse(base_dir *parent);
	private:
		void get_chunks();
		std::string &front();
		std::string pop_front();
		bool erase_chunk_middle(std::string str, bool);
		bool erase_chunk_front(std::string str);
		void erase_token_front(std::string token, bool);
		bool is_context(std::string context);
		bool is_directive(std::string directive);
		bool is_response_code(unsigned int response_code);
		base_dir *process_http(base_dir*);
		base_dir *process_server(base_dir *http);
		base_dir *process_location(base_dir *server);
		unsigned int strtoul(const std::string &number);
		bool	read_root(base_dir *parent);
		bool	read_autoindex(base_dir *parent);
		bool	read_error_page(base_dir *parent);
		bool	read_client_max_body_size(base_dir *parent);
		bool	read_index(base_dir *parent);
		bool	read_redirect(base_dir *parent);
		bool	read_listen(base_dir *server);
		bool	read_server_name(base_dir *server);
		bool	read_cgi(base_dir *location);
		bool	read_limit_except(base_dir *location);
		void	push_brace(char brace);
		void	load_base_dir();
		void	unload_base_dir();
};

#endif
