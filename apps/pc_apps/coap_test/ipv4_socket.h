#ifndef IPV4_SOCKET_H_
#define IPV4_SOCKET_H_

#include <netinet/in.h>

class IPv4Socket
{
public:
	IPv4Socket()
	{
		s_port = 0;
		s_addr = INADDR_ANY;
	}

	~IPv4Socket()
	{

	}

	IPv4Socket( in_addr_t addr, in_port_t port )
	{
		s_port = port;
		s_addr = addr;
	}

	bool operator==( const IPv4Socket &other ) const
	{
		return ( this->s_addr == other.s_addr && this->s_port == other.s_port );
	}

	bool operator!=( const IPv4Socket &other ) const
	{
		return !(*this == other);
	}

	operator int()
	{
		return ( (int) s_addr );
	}

	operator uint32_t()
	{
		return ( (uint32_t) s_addr );
	}

	IPv4Socket& operator=( const IPv4Socket &rhs )
	{
		// avoid self-assignment
		if(this != &rhs)
		{
			this->s_addr = rhs.s_addr;
			this->s_port = rhs.s_port;
		}

		return *this;
	}

	in_addr_t addr() const
	{
		return s_addr;
	}

	void set_addr( in_addr_t addr )
	{
		s_addr = addr;
	}

	in_port_t port() const
	{
		return s_port;
	}

	void set_port( in_port_t port )
	{
		s_port = port;
	}


private:
	in_addr_t s_addr;
	in_port_t s_port;
};

#endif /* IPV4_SOCKET_H_ */
