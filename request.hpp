#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "stdafx.hpp"

namespace ft
{
    class request 
    {
        private:
            std::string method;
            std::string uri;
            string_map headers;
            std::string body;

        public:
            request(const std::string &request);
    };
}

#endif