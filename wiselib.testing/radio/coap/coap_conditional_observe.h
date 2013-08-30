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

static const int COAP_MAX_HL_STATES = 10;
static const int COAP_MAX_STATE_RESOURCES = 10;

typedef struct observer<uint16_t> observer_t;

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
namespace ConditionalObserveValueType {
	enum ObserveValueTye {
		INTEGER,
		DURATION_S,
		FLOAT
	};
}
typedef ConditionalObserveValueType::ObserveValueTye condition_value_type_t;
// --------------------------------------------------------------------------
struct coap_condition
{
	condition_type_t type;
	condition_reliability_t reliability;
	condition_value_type_t value_type;

};

template<typename V_Type, typename Value_T>
bool coap_satisfies_condition(
		Value_T new_value,
		observer_t observer,
		uint32_t time)
{
	switch (observer.condition.type) {
		case ConditionalObserveType::CANCELLATION:

			break;
		case ConditionalObserveType::TIMESERIES:
			return observer.last_value != new_value;

		case ConditionalObserveType::MINIMUM_RESPONSE_TIME:
			return ( observer.last_value != new_value ) && ( c_value <= ( time - observer.timestamp) );

		case ConditionalObserveType::MAXIMUM_RESPONSE_TIME:
			return ( observer.last_value != new_value ) || ( c_value <= ( time - observer.timestamp ) );

		case ConditionalObserveType::STEP:
			return abs<V_Type>(observer.last_value - new_value) >= c_value;

		case ConditionalObserveType::ALLVALUES_SMALLER:
			return new_value < c_value;

		case ConditionalObserveType::ALLVALUES_GREATER:
			return new_value > c_value;

		case ConditionalObserveType::VALUE_EQUALS:
			return new_value == c_value;

		case ConditionalObserveType::VALUE_DIFFERS:
			return new_value != c_value;

		case ConditionalObserveType::PERIODIC:
			return ( ( time - observer.timestamp ) >= c_value );

		default:
			break;
	}
}

coap_condition coap_parse_condition( OpaqueData raw )
{
	// TODO parse condition
}

/*
template <typename message_t>
coap_condition parse_message(message_t msg)
{
	value_t lower = 0 | (option[1]<<8) | option[2];
	value_t upper = 0 | (option[3]<<8) | option[4];
	char* name = (char*) option+5;
	char* name_copy = new char[hl_data.length()-5+1];
	memcpy(name_copy, name, hl_data.length()-5);
	cout << "Created integer state:\"" << name << "\" lower: " << lower << " upper: " << upper << "\n";

	const number_state<value_t> state = {lower,upper,name_copy};

}
*/

template<typename T> inline const T abs(T const & x)
{
    return ( x<0 ) ? -x : x;
}


#endif /* COAP_CONDITIONAL_OBSERVE_H_ */
