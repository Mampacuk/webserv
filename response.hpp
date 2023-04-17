#ifndef RESPONSE_HPP
# define RESPONSE_HPP

namespace ft
{
	class response
	{
		private:
			int _socket;
			//...
		public:
			response();
			~response();
			response(const response &other);
			response &operator=(const response &other);
			int get_socket() const;
			operator int() const;
			//...
	};
}

#endif
