#include "base_dir.hpp"

namespace ft
{
	base_dir::base_dir() : _autoindex(false), _root("html"), _cgi_executable(), _cgi_extension(), _client_max_body_size(1000000), _error_pages(), _indices(), _flush_error_pages(false), _flush_indices(false)
	{
		_indices.push_back("index");
		std::cout << LGREEN "pushed str " YELLOW << _indices.back() << LGREEN " under address " MAGENTA << &_indices.back() << RESET << std::endl;
		_indices.push_back("index.html");
		std::cout << LGREEN "pushed str " YELLOW << _indices.back() << LGREEN " under address " MAGENTA << &_indices.back() << RESET << std::endl;
	}

	base_dir::~base_dir() {}

	base_dir::base_dir(const base_dir &other) : _autoindex(other._autoindex), _root(other._root), _cgi_executable(other._cgi_executable), _cgi_extension(other._cgi_extension), _client_max_body_size(other._client_max_body_size), _error_pages(other._error_pages), _indices(other._indices), _flush_error_pages(false), _flush_indices(false)
	{
		std::cout << "other under " << &other << std::endl;
		std::cout << "other's indices size is " << other._indices.size() << std::endl;
		std::cout << "other's indices are under " << &other._indices << std::endl;
		std::cout << "this indices size is " << _indices.size() << std::endl;
		std::cout << "this indices are under " << &_indices << std::endl;
		std::cout << "this is under " << this << std::endl;
	}

	base_dir &base_dir::operator=(const base_dir &other)
	{
		_autoindex = other._autoindex;
		_root = other._root;
		_cgi_executable = other._cgi_executable;
		_cgi_extension = other._cgi_extension;
		_client_max_body_size = other._client_max_body_size;
		_error_pages = other._error_pages;
		std::cout << MAGENTA "other under address " << &other << RESET << std::endl;
		std::cout << MAGENTA "its indices are " << other._indices.size() << RESET << std::endl;
		_indices = other._indices;
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

	void base_dir::set_cgi_executable(const std::string &cgi_executable)
	{
		_cgi_executable = cgi_executable;
	}

	void base_dir::set_cgi_extension(const std::string &cgi_extension)
	{
		_cgi_extension = cgi_extension;
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

	const std::string &base_dir::get_cgi_executable() const
	{
		return (_cgi_executable);
	}

	const std::string &base_dir::get_cgi_extension() const
	{
		return (_cgi_extension);
	}

	void base_dir::add_error_page(unsigned int error_code, const std::string &page)
	{
		_error_pages[error_code] = starts_with(page, "/") ? page.substr(1, page.size()) : page;
	}

	void base_dir::add_index(const std::string &index_file)
	{
		_indices.push_back(starts_with(index_file, "/") ? index_file.substr(1, index_file.size()) : index_file);
	
	}

	std::string base_dir::get_error_page(unsigned int error_code) const
	{
		error_map::const_iterator it;
		
		it = _error_pages.find(error_code);
		return (it != _error_pages.end() ? it->second : "");
	}

	const string_vector &base_dir::get_indices() const
	{
		return (_indices);
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
			std::cout << CYAN "ABOUT TO FLUSH INDICES OF SIZE " << _indices.size() << RESET << std::endl;
			// for (size_t i = 0; i < _indices.size(); i++)
			// 	std::cout << MAGENTA "index " << i << ": " << _indices[i] << RESET << std::endl;

			_indices.clear();
			std::cout << CYAN "FLUSHED INDICES" RESET << std::endl;
			_flush_indices = true;
		}
	}
}
