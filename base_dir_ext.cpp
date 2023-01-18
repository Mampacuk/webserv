#include "base_dir_ext.hpp"

base_dir_ext::base_dir_ext(): base_dir(), locations(), redirects() {}

base_dir_ext::~base_dir_ext() {}

base_dir_ext::base_dir_ext(const base_dir_ext &other): base_dir(other), locations(other.locations), redirects(other.redirects) {}

base_dir_ext::base_dir_ext(const base_dir &other): base_dir(other), locations(), redirects() {}

base_dir_ext &base_dir_ext::operator=(const base_dir_ext &other)
{
    base_dir::operator=(other);
    this->locations = other.locations;
    this->redirects = other.redirects;
}

const std::multimap<std::string, std::string> &base_dir_ext::get_redirects() const
{
    return (this->redirects);
}

const std::set<location> &base_dir_ext::get_locations() const
{
    return (this->locations);
}

void base_dir_ext::add_redirect(std::string expression, std::string uri)
{
    this->redirects.insert(expression, uri);
}

void base_dir_ext::add_location(location location)
{
    this->locations.insert(location);
}
