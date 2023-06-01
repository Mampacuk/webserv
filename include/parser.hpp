#ifndef PARSER_HPP
# define PARSER_HPP

# include "base_dir.hpp"
# include "webserv.hpp"

namespace ft
{
	class parser
	{
		private:
			typedef std::pair<bool (parser::*)(base_dir*), bool>		dir_functor;
			typedef std::pair<base_dir *(parser::*)(base_dir*), bool>	cont_functor;

			typedef std::pair<std::string, dir_functor>		directive;
			typedef std::map<std::string, dir_functor>		directive_map;
			typedef std::pair<std::string, cont_functor>	context;
			typedef std::map<std::string, cont_functor>		context_map;

			directive_map					_directives;
			context_map						_contexts;
			std::ifstream					_config;
			string_list						_chunks;
			string_pair_set					_listens;
			string_pair_server_pointer_mmap	_sockets;
		private:
			parser();
			parser(const parser&);
			parser &operator=(const parser&);
		public:
			~parser();
			parser(const std::string &filename);
			base_dir *parse(base_dir *parent = NULL);
		private:
			void parse_chunks();
			bool is_context(std::string context);
			bool is_directive(std::string directive);
			base_dir *process_http(base_dir*);
			base_dir *process_server(base_dir *http);
			base_dir *process_location(base_dir *server);
			bool read_autoindex(base_dir *parent);
			bool read_root(base_dir *parent);
			bool read_client_max_body_size(base_dir *parent);
			bool read_cgi(base_dir *parent);
			bool read_error_page(base_dir *parent);
			bool read_index(base_dir *parent);
			bool read_rewrite(base_dir *parent);
			bool read_listen(base_dir*);
			bool read_server_name(base_dir *server);
			bool read_limit_except(base_dir *location);
			std::string &front();
			std::string pop_front();
			string_vector get_argument_list();
			void memorize_listen(const std::string &host, const std::string &port);
			void map_sockets(const server *server);
			void open_sockets(http *protocol);
			bool erase_chunk_front(std::string str);
			bool erase_chunk_middle(std::string str, bool b = false);
			void erase_token_front(std::string token, bool);
			void load_base_dir();
			void unload_base_dir();
	};
}

#endif
