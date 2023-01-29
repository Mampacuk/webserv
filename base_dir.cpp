#include "base_dir.hpp"

base_dir::base_dir(): root("html"), autoindex(false), error_pages(), client_max_body_size(1), indexes() {}

base_dir::~base_dir() {}

base_dir::base_dir(const base_dir &other): root(other.root), autoindex(other.autoindex), error_pages(other.error_pages), client_max_body_size(other.client_max_body_size), indexes(other.indexes) {}

base_dir &base_dir::operator=(const base_dir &other)
{
	this->root = other.root;
	this->autoindex = other.autoindex;
	this->client_max_body_size = other.client_max_body_size;
	this->error_pages = other.error_pages;
	this->indexes = other.indexes;
}

void base_dir::set_root(const std::string &root)
{
	this->root = root;
}

void base_dir::set_autoindex(bool autoindex)
{
	this->autoindex = autoindex;
}

void base_dir::set_client_max_body_size(unsigned long int size)
{
	this->client_max_body_size = size;
}

const std::string &base_dir::get_root() const
{
	return (this->root);
}

bool base_dir::get_autoindex() const
{
	return (this->autoindex);
}

unsigned int base_dir::get_client_max_body_size() const
{
	return (this->client_max_body_size);
}

void base_dir::add_error_page(unsigned int error_code, const std::string &error_page)
{
	this->error_pages.insert(std::pair<unsigned int, std::string>(error_code, error_page));
}

void base_dir::add_index(const std::string &index_file)
{
	this->indexes.push_back(index_file);
}

const std::string base_dir::get_error_page(unsigned int error_code) const
{
	std::map<unsigned int, std::string>::const_iterator it;
	
	it = error_pages.find(error_code);
	if (it != error_pages.end())
		return (it->second);
	return ("");
}

const std::vector<std::string> &base_dir::get_indexes() const
{
	return (this->indexes);
}









