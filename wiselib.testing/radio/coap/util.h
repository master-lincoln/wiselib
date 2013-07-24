/*
 * util.h
 *
 *  Created on: Jul 24, 2013
 *      Author: wiselib
 */

#ifndef UTIL_H_
#define UTIL_H_

namespace wiselib {

enum path_compare
{
	EQUAL,
	LHS_IS_SUBRESOURCE,
	RHS_IS_SUBRESOURCE,
	NOT_EQUAL
};

template<typename string_t>
int path_cmp(const string_t &lhs, const string_t &rhs)
{
	for( size_t i = 0; ; ++i )
	{
		if( i == lhs.length() )
		{
			if( i == rhs.length() )
				return EQUAL;
			else if( rhs[i] == '/' )
				return RHS_IS_SUBRESOURCE;
			else
				return NOT_EQUAL;
		}
		if( i == rhs.length() )
		{
			if( lhs[i] == '/' )
				return LHS_IS_SUBRESOURCE;
			else
				return NOT_EQUAL;
		}

		if( lhs[i] != rhs[i] )
			return NOT_EQUAL;
	}
}

}



#endif /* UTIL_H_ */
