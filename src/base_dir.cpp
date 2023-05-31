#include "base_dir.hpp"

namespace ft
{
	base_dir::base_dir() : _autoindex(false), _root("html"), _client_max_body_size(1000000), _cgi(), _error_pages(), _indices(), _flush_cgi(false), _flush_error_pages(false), _flush_indices(false)
	{
		_indices.push_back("index");
		_indices.push_back("index.html");
	}

	base_dir::~base_dir() {}

	base_dir::base_dir(const base_dir &other) : _autoindex(other._autoindex), _root(other._root), _client_max_body_size(other._client_max_body_size), _cgi(other._cgi), _error_pages(other._error_pages), _indices(other._indices), _flush_cgi(false), _flush_error_pages(false), _flush_indices(false) {}

	base_dir &base_dir::operator=(const base_dir &other)
	{
		_autoindex = other._autoindex;
		_root = other._root;
		// _cgi_executable = other._cgi_executable;
		// _cgi_extension = other._cgi_extension;
		_client_max_body_size = other._client_max_body_size;
		_cgi = other._cgi;
		_error_pages = other._error_pages;
		_indices = other._indices;
		_flush_cgi = other._flush_cgi;
		_flush_error_pages = other._flush_error_pages;
		_flush_indices = other._flush_indices;
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

	void base_dir::add_cgi(const std::string &cgi_extension, const std::string &cgi_executable)
	{
		_cgi.insert(string_pair(cgi_extension, cgi_executable));
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

	std::string base_dir::get_cgi_executable(const std::string &cgi_extension) const
	{
		string_map::const_iterator it;
		
		std::cout << "Cgi extension in get_cgi_executable: " << cgi_extension << std::endl;
		std::cout << "Cgi map size: " << _cgi.size() << std::endl;
		it = _cgi.find(cgi_extension);	
		return (it != _cgi.end() ? it->second : "");
	}

	void base_dir::add_error_page(http_code error, const std::string &page)
	{
		// _error_pages[error] = starts_with(page, "/") ? page.substr(1, page.size()) : page;
		_error_pages.insert(std::make_pair(error, (!starts_with(page, "/") ? "/" : "") + page));
	}

	void base_dir::add_index(const std::string &index_file)
	{
		_indices.push_back(starts_with(index_file, "/") ? index_file.substr(1, index_file.size()) : index_file);
	
	}

	std::string base_dir::get_error_page(http_code error) const
	{
		error_map::const_iterator it;
		
		it = _error_pages.find(error);
		return (it != _error_pages.end() ? it->second : "");
	}

	const string_vector &base_dir::get_indices() const
	{
		return (_indices);
	}

	void base_dir::flush_cgi()
	{
		if (!_flush_cgi)
		{
			_cgi.clear();
			_flush_cgi = true;
		}
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

	const string_map &base_dir::get_cgi() const
	{
		return (_cgi);
	}
}
