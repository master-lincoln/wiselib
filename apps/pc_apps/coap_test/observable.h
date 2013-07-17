/*
 * observable.h
 *
 *  Created on: Jul 13, 2013
 *      Author: wiselib
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "radio/coap/coap.h"
#include "util/pstl/list_static.h"
#include "observer.h"

namespace wiselib
{

template<typename OsModel_P, typename Value_T>
class Observable
{
public:
	typedef OsModel_P Os;
	typedef Value_T value_t;
	typedef IObserver<value_t> observer_t;

	virtual ~Observable() { }

	virtual void notify_observers(value_t value) {
		// call notify(value) on each observer
		for ( size_t i=0; i<observers_.size(); ++i)
		{
			observers_.at(i)->notify(value);
		}
	}

	void add_observer(observer_t& observer) {
			observers_.push_front(&observer);
		}

	void remove_observer(observer_t& observer) {
		observers_.remove(observer);
	}

	void remove_all_observers() {
		observers_.clear();
	}

private:

	list_static<Os, observer_t*, COAP_MAX_OBSERVERS> observers_;

};

}

#endif /* OBSERVABLE_H_ */
