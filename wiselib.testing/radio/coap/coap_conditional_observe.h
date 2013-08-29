/*
 * coap_high_level_states.h
 *
 *  Created on: Jul 23, 2013
 *      Author: wiselib
 */

#ifndef COAP_CONDITIONAL_OBSERVE_H_
#define COAP_CONDITIONAL_OBSERVE_H_

#include "util/pstl/vector_static.h"
#include "observable_service.h"

namespace wiselib {

typedef observer observer_t;


static const int COAP_MAX_HL_STATES = 10;
static const int COAP_MAX_STATE_RESOURCES = 10;

// --------------------------------------------------------------------------
namespace ConditionalObserveType {
	enum ConditionType {
		CANCELLATION,
		TIMESERIES,
		MINIMUM_RESPONSE_TIME,
		MAXIMUM_RESPONSE_TIME,
		STEP,
		ALLVALUES_SMALLER,
		ALLVALUES_GREATER,
		VALUE_EQUALS,
		VALUE_DIFFERS,
		PERIODIC
	};
	static const bool HasConditionValue[10] =
	{
		false,			// 0: CANCELLATION
		false,			// 1: TIMESERIES
		true,			// 2: MINIMUM_RESPONSE_TIME
		true,			// 3: MAXIMUM_RESPONSE_TIME
		true,			// 4: STEP
		true,			// 5: ALLVALUES_SMALLER
		true,			// 6: ALLVALUES_GREATER
		true,			// 7: VALUE_EQUALS
		true,			// 8: VALUE_DIFFERS
		true,			// 9: PERIODIC
	};
}
typedef ConditionalObserveType::ConditionType condition_type_t;
// --------------------------------------------------------------------------
namespace ConditionalObserveReliability {
	enum ObserveReliability {
		NON,
		CON
	};
}
typedef ConditionalObserveReliability::ObserveReliability condition_reliability_t;
// --------------------------------------------------------------------------
struct coap_condition
{
	condition_type_t type;
	condition_reliability_t reliability;

};

template<typename V_Type, typename Value_T>
bool coap_is_condition_met(
		Value_T new_value,
		condition_type_t c_type,
		V_Type c_value,
		observer_t observer)
{
	switch (c_type) {
		case ConditionalObserveType::CANCELLATION:

			break;
		case ConditionalObserveType::TIMESERIES:

			break;
		case ConditionalObserveType::MINIMUM_RESPONSE_TIME:

			break;
		case ConditionalObserveType::MAXIMUM_RESPONSE_TIME:

			break;
		case ConditionalObserveType::STEP:
			return abs<V_Type>(observer.last_value - new_value) > c_value;

		case ConditionalObserveType::ALLVALUES_SMALLER:
			return new_value < c_value;

		case ConditionalObserveType::ALLVALUES_GREATER:
			return new_value > c_value;

		case ConditionalObserveType::VALUE_EQUALS:
			return new_value == c_value;

		case ConditionalObserveType::VALUE_DIFFERS:
			return new_value != c_value;

		case ConditionalObserveType::PERIODIC:

			break;
		default:
			break;
	}
}

template<typename T> inline const T abs(T const & x)
{
    return ( x<0 ) ? -x : x;
}


#endif /* COAP_CONDITIONAL_OBSERVE_H_ */
