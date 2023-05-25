#include "base_dir.hpp"

namespace ft
{
	base_dir::base_dir() : _autoindex(false), _root("html"), _client_max_body_size(1000000), _error_pages(), _indices(), _cgi_params(), _flush_error_pages(false), _flush_indices(false), _flush_cgi_params(false)
	{
		_indices.push_back("index");
		_indices.push_back("index.html");
	}

	base_dir::~base_dir() {}

	base_dir::base_dir(const base_dir &other) : _autoindex(other._autoindex), _root(other._root), _client_max_body_size(other._client_max_body_size), _error_pages(other._error_pages), _indices(other._indices), _cgi_params(other._cgi_params), _flush_error_pages(false), _flush_indices(false), _flush_cgi_params(false) {}

	base_dir &base_dir::operator=(const base_dir &other)
	{
		_autoindex = other._autoindex;
		_root = other._root;
		_client_max_body_size = other._client_max_body_size;
		_error_pages = other._error_pages;
		_indices = other._indices;
		_cgi_params = other._cgi_params;
		_flush_error_pages = other._flush_error_pages;
		_flush_indices = other._flush_indices;
		_flush_cgi_params = other._flush_cgi_params;
		return (*this);
	}

	void base_dir::set_root(const std::string &root)
	{
		_root = (ends_with(root, "/") ? root.substr(0, root.size() - 1) : root);
	}

	void base_dir::set_autoindex(bool autoindex)
	{
		_autoindex = autoindex;
	}

	void base_dir::set_client_max_body_size(unsigned long int size)
	{
		_client_max_body_size = size;
	}

	const std::string &base_dir::get_root() const
	{
		return (_root);
	}

	bool base_dir::get_autoindex() const
	{
		return (_autoindex);
	}

	unsigned int base_dir::get_client_max_body_size() const
	{
		return (_client_max_body_size);
	}

	void base_dir::add_error_page(unsigned int error_code, const std::string &page)
	{
		_error_pages[error_code] = starts_with(page, "/") ? page.substr(1, page.size()) : page;
	}

	void base_dir::add_index(const std::string &index_file)
	{
		_indices.push_back(starts_with(index_file, "/") ? index_file.substr(1, index_file.size()) : index_file);
	
	}

	void base_dir::add_cgi_param(const std::string &key, const std::string &value)
	{
		_cgi_params.insert(string_pair(key, value));
	}

	std::string base_dir::get_error_page(unsigned int error_code) const
	{
		error_map::const_iterator it;
		
		it = _error_pages.find(error_code);
		return (it != _error_pages.end() ? it->second : "");
	}

	std::string base_dir::get_cgi_param(const std::string &key) const
	{
		string_map::const_iterator it;

		it = _cgi_params.find(key);
		return (it != _cgi_params.end() ? it->second : "");
	}

	const string_vector &base_dir::get_indices() const
	{
		return (_indices);
	}

	const string_map &base_dir::get_cgi_params() const
	{
		return (_cgi_params);
	}

	void base_dir::flush_error_pages()
	{
		if (!_flush_error_pages)
		{
			_error_pages.clear();
			_flush_error_pages = true;
		}
	}

	void base_dir::flush_indices()
	{
		if (!_flush_indices)
		{
			_indices.clear();
			_flush_indices = true;
		}
	}

	void base_dir::flush_cgi_params()
	{
		if (!_flush_cgi_params)
		{
			_cgi_params.clear();
			_flush_cgi_params = true;
		}
	}
}
