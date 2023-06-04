# webserv
## Mandatory Part
You must write a HTTP server in C++ 98.

Your executable will be run as follows:
```
./webserv [configuration file]
```
## Features and Requirements
*	The server supports GET, POST and DELETE requests.
*	The server supports CGI scripts that are not NPH (non-parsed header).
	*	The script _MUST_ output a double CRLF sequence, `\r\n\r\n`. If it's not output by the script, the server will add it itself and potentially ruin headers added by the CGI.
	*	Similarly, if the server detects an unintended double CRLF, it won't add one itself, potentially letting the content remain in the headers section.
*	HTTP redirection is implemented _internally_.
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
*	The server is not working in a round-robin fashion (which is preferable) due the responses and the requests being held by value, not by pointer. Therefore, it's preferable to have a `response_pointer_list` and `request_pointer_list` instead of `response_list` and `request_list` to be able to freely push semi-received/sent requests/responses to the back of the queue without copying.
*	Since `getenv()` and `getcwd()` are no longer allowed by the subject, `DOCUMENT_ROOT` environment variable to CGI is a _relative_ path instead of absolute: a workaround is to take in environ as `main()`'s third argument and grab the value of `PWD` as the absolute document root.
*	Since `usleep()` is no longer allowed by the subject, we are yet to see a portable and interoperable way to create a time-out mechanism for CGI scripts that returns the "504 Gateway Timeout" error.
## Credits
Team "The It":
*	Anahit Apresyan | aapresya: parsing, design & class hierarchy, responses (GET method), testing & quality assurance
*	Alexander Israelyan | aisraely: parsing, design & class hierarchy, requests, responses (POST, DELETE methods), testing & quality assurance