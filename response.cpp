#include "response.hpp"

namespace ft
{
	// response::response() {}
	
	response::~response() {}

	response::operator int() const
	{
		return (_req._socket);
	}


}