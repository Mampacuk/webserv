#include "base_dir.hpp"

namespace ft
{
	base_dir::base_dir(): _root("html"), _autoindex(false), _error_pages(), _client_max_body_size(1000000), _indexes() {}

	base_dir::~base_dir() {}

	base_dir::base_dir(const base_dir &other): _root(other._root), _autoindex(other._autoindex), _error_pages(other._error_pages), _client_max_body_size(other._client_max_body_size), _indexes(other._indexes) {}

	base_dir &base_dir::operator=(const base_dir &other)
	{
		this->_root = other._root;
		this->_autoindex = other._autoindex;
		this->_client_max_body_size = other._client_max_body_size;
		this->_error_pages = other._error_pages;
		this->_indexes = other._indexes;
		return (*this);
	}

	void base_dir::set_root(const std::string &_root)
	{
		this->_root = _root;
	}

	void base_dir::set_autoindex(bool _autoindex)
	{
		this->_autoindex = _autoindex;
	}

	void base_dir::set_client_max_body_size(unsigned long int size)
	{
		this->_client_max_body_size = size;
	}

	const std::string &base_dir::get_root() const
	{
		return (this->_root);
	}

	bool base_dir::get_autoindex() const
	{
		return (this->_autoindex);
	}

	unsigned int base_dir::get_client_max_body_size() const
	{
		return (this->_client_max_body_size);
	}

	void base_dir::add_error_page(unsigned int error_code, const std::string &page)
	{
		this->_error_pages[error_code] = page;
	}

	void base_dir::add_index(const std::string &index_file)
	{
		this->_indexes.push_back(index_file);
	}

	const std::string base_dir::get_error_page(unsigned int error_code) const
	{
		std::map<unsigned int, std::string>::const_iterator it;
		
		it = _error_pages.find(error_code);
		if (it != _error_pages.end())
			return (it->second);
		return ("");
	}

	const string_vector &base_dir::get_indexes() const
	{
		return (this->_indexes);
	}

	void base_dir::flush_error_pages()
	{
		this->_error_pages.clear();
	}

	void base_dir::flush_indexes()
	{
		this->_indexes.clear();
	}
}
