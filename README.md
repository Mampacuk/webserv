# webserv
## Mandatory Part
You must write a HTTP server in C++ 98.

Your executable will be run as follows:
```
./webserv [configuration file]
```
## Configuration File
Server configuration file is by default searched at `conf/webserv.conf`, otherwise is taken as a program argument.

The file is parsed just as an ordinary nginx configuration file, with certain exceptions:
*	Options for directives are not supported.
*	Regular expressions are not supported.
*	Escape characters and quotes are not supported.
*	The file is parsed from top to bottom, hence directives are inherited from top to bottom; directives specified above a lower context will be inherited, while directives specified below a lower context **will not** be inherited.
*	`events` context is neither supported nor required.
*	`fastcgi_pass`, `fastcgi_param` are replaced by `cgi` directive that works in the following way:
	```
	cgi abs_path_to_bin extension_without_dot;
	```
*	`limit_except` is a directive, not a context. It gets methods in an argument list (much like `index`).
## Known Issues
Please be welcome to create pull requests to solve the following issues:
*	There's a heisenbug in "parser.cpp" file on Line 138 that from time to time throws "_*** malloc trying to deallocate not allocated memory_" when `server::flush_indices()` is called upon a `server::server(const base_dir&)` instantiation. Can be relieved by using `std::cout` statements around those lines.