# webserv
## Config Parser
Server configuration file is by default searched at `conf/webserv.conf`, otherwise is taken as a program argument.

The file is parsed just as an ordinary nginx configuration file, with certain exceptions:
*	Regular expressions are not supported.
*	Escape characters and quotes are not supported.
*	The file is parsed from top to bottom, hence directives are inherited from top to bottom; directives specified above a lower context will be inherited, while directives specified below a lower context won't be inherited.
*	`events` context is neither supported nor required.
*	`fastcgi_pass`, `fastcgi_param` are replaced by `cgi` directive.
	*	There can only be _one_ CGI served at a location at a time.
*	`limit_except` is a directive, not a context. It gets methods in an argument list (much like `index`).
*	Options for directives are not supported.