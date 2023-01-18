#include "base_dir.hpp"

base_dir::base_dir(): root("html"), autoindex(false), error_pages(), 
						client_max_body_size(1), indexes() {}

base_dir::~base_dir() {}

base_dir::base_dir(const base_dir &other)
{
	*this = other;
}

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

void base_dir::set_client_max_body_size(unsigned int size)
{
	
}

