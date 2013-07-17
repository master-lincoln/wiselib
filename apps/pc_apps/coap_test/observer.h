/*
 * observer.h
 *
 *  Created on: Jul 13, 2013
 *      Author: wiselib
 */

#ifndef OBSERVER_H_
#define OBSERVER_H_


namespace wiselib
{

template<typename Value_T>
class IObserver
{
public:
	typedef Value_T value_t;

	virtual ~IObserver();
	virtual void notify(value_t value) = 0;

};

}

#endif /* OBSERVER_H_ */
