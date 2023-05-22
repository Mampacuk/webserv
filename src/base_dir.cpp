#include "base_dir.hpp"

namespace ft
{
	base_dir::base_dir() : _autoindex(false), _error_pages(), _root("html"), _indexes(), _client_max_body_size(1000000), _cgi_params()
	{
		_indexes.push_back("index");
		_indexes.push_back("index.html");
	}

	base_dir::~base_dir() {}

	base_dir::base_dir(const base_dir &other) : _autoindex(other._autoindex), _error_pages(other._error_pages), _root(other._root), _indexes(other._indexes), _client_max_body_size(other._client_max_body_size), _cgi_params(other._cgi_params) {}

	base_dir &base_dir::operator=(const base_dir &other)
	{
		this->_root = other._root;
		this->_autoindex = other._autoindex;
		this->_client_max_body_size = other._client_max_body_size;
		this->_error_pages = other._error_pages;
		this->_indexes = other._indexes;
		this->_cgi_params = other._cgi_params;
		return (*this);
	}

	void base_dir::set_root(const std::string &root)
	{
		this->_root = (ends_with(root, "/") ? root : root + "/");
	}

	void base_dir::set_autoindex(bool autoindex)
	{
		this->_autoindex = autoindex;
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

	void base_dir::add_cgi_param(const std::string &key, const std::string &value)
	{
		this->_cgi_params.insert(string_pair(key, value));
	}

	std::string base_dir::get_error_page(unsigned int error_code) const
	{
		error_map::const_iterator it;
		
		it = this->_error_pages.find(error_code);
		return (it != this->_error_pages.end() ? it->second : "");
	}

	std::string base_dir::get_cgi_param(const std::string &key) const
	{
		string_map::const_iterator it;

		it = this->_cgi_params.find(key);
		return (it != this->_cgi_params.end() ? it->second : "");
	}

	const string_vector &base_dir::get_indexes() const
	{
		return (this->_indexes);
	}

	const string_map &base_dir::get_cgi_params() const
	{
		return (this->_cgi_params);
	}

	void base_dir::flush_error_pages()
	{
		this->_error_pages.clear();
	}

	void base_dir::flush_indexes()
	{
		this->_indexes.clear();
	}

	void base_dir::flush_cgi_params()
	{
		this->_cgi_params.clear();
	}
}
