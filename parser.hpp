#ifndef PARSER_HPP
# define PARSER_HPP

# include "base_dir.hpp"
# include <stack>
# include <fstream>
# include <list>

class parser
{
	public:
		typedef std::map<std::string, void (parser::*)(base_dir*)> directive_map;
		typedef std::map<std::string, base_dir *(parser::*)(base_dir*)> context_map;
	private:
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
		bool get_chunks();
		bool is_context(std::string context);
		bool is_directive(std::string context);
};

#endif