#ifndef PARSER_HPP
# define PARSER_HPP

# include "stdafx.hpp"
# include "base_dir.hpp"

namespace ft
{
	class parser
	{
		private:
			typedef std::pair<bool (parser::*)(base_dir*), bool> dir_functor;
			typedef std::pair<base_dir *(parser::*)(base_dir*), bool> cont_functor;

			typedef std::pair<std::string, dir_functor> directive;
			typedef std::map<std::string, dir_functor> directive_map;
			typedef std::pair<std::string, cont_functor> context;
			typedef std::map<std::string, cont_functor> context_map;

			directive_map directives;
			context_map contexts;
			std::ifstream config;
			string_list chunks;
			string_pair_set listens;
		public:
			parser(const std::string &filename);
			~parser();
			base_dir *parse(base_dir *parent = NULL);
		private:
			parser();
			parser(const parser &other);
			parser &operator=(const parser &other);
			void parse_chunks();
			std::string &front();
			std::string pop_front();
			string_vector get_argument_list();
			void memorize_listen(const std::string &host, const std::string &port);
			void open_sockets(server &server);
			bool erase_chunk_middle(std::string str, bool b = false);
			bool erase_chunk_front(std::string str);
			void erase_token_front(std::string token, bool);
			bool is_context(std::string context);
			bool is_directive(std::string directive);
			bool is_response_code(unsigned int response_code);
			bool is_port_number(const std::string &port_string);
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
			void	load_base_dir();
			void	unload_base_dir();
	};
}

#endif
