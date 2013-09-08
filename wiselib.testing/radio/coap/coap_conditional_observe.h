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

#define DEBUG_OBSERVE

namespace wiselib {

static const int COAP_MAX_CONDITIONS = 10;

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
	enum ObserveValueType {
		INTEGER,
		DURATION_S,
		FLOAT
	};
}
typedef ConditionalObserveValueType::ObserveValueType condition_value_type_t;
// --------------------------------------------------------------------------
struct coap_condition
{
	condition_type_t type;
	condition_reliability_t reliability;
	condition_value_type_t value_type;
	uint32_t condition_value_raw;
};

template<typename CValue_T>
CValue_T convert_condition_value(uint32_t raw, ConditionalObserveValueType::ObserveValueType type)
{
	if ( type == ConditionalObserveValueType::DURATION_S ) {
		// convert from duration type defined in Appendix C.2 of draft-bormann-coap-misc-13
		const uint8_t N = 8;
		const uint8_t E = 4;
		const uint8_t HIBIT = (1 << (N - 1));
		const uint8_t EMASK = ((1 << E) - 1);
		const uint8_t MMASK = ((1 << N) - 1 - EMASK);
		return (raw < HIBIT ? raw : (raw & MMASK) << (raw & EMASK));
	}
	return (CValue_T) raw;
}

template<>
float convert_condition_value(uint32_t raw, ConditionalObserveValueType::ObserveValueType type)
{
	if ( type == ConditionalObserveValueType::FLOAT )
	{
		// TODO this is probably not the float representation we want
		float f;
		memcpy(&f, &raw, sizeof(f));
		return f;
	}
	else
	{
		// TODO error message in debug
		return -1;
	}
}


template<typename Value_T, typename Observer_T, typename CValue_T>
bool coap_satisfies_condition(
		Value_T new_value,
		Observer_T& observer,
		coap_condition& condition,
		uint32_t time)
{
	typedef Value_T value_t;

	CValue_T condition_value = convert_condition_value<CValue_T>(condition.condition_value_raw, condition.value_type);
	bool satisfied = false;

	switch (condition.type) {
		case ConditionalObserveType::CANCELLATION:
			// TODO implement cancellation
			break;

		case ConditionalObserveType::TIMESERIES:
			satisfied = observer.last_value != new_value;
			break;

		case ConditionalObserveType::MINIMUM_RESPONSE_TIME:
			// TODO what if time wraps?
			satisfied = ( observer.last_value != new_value ) && ( condition_value <= ( time - observer.timestamp) );
			break;

		case ConditionalObserveType::MAXIMUM_RESPONSE_TIME:
			satisfied = ( observer.last_value != new_value ) || ( condition_value <= ( time - observer.timestamp ) );
			break;

		case ConditionalObserveType::STEP:
			satisfied = abs(observer.last_value - new_value) >= condition_value;
			//printf("COND_OBS: STEP from %d to %d is %s than %d\n",
			//			observer.last_value,
			//			new_value,
			//			( satisfied ? "bigger" : "smaller" ),
			//			condition_value);
			break;

		case ConditionalObserveType::ALLVALUES_SMALLER:
			satisfied = new_value < condition_value;
			break;

		case ConditionalObserveType::ALLVALUES_GREATER:
			satisfied = new_value > condition_value;
			break;

		case ConditionalObserveType::VALUE_EQUALS:
			satisfied = new_value == condition_value;
			break;

		case ConditionalObserveType::VALUE_DIFFERS:
			satisfied = new_value != condition_value;
			break;

		case ConditionalObserveType::PERIODIC:
			satisfied = ( ( time - observer.timestamp ) >= condition_value );
			break;
	}
	return satisfied;
}

template<typename condition_list_t, typename Value_T, typename Observer_T>
bool coap_satisfies_conditions(
		Value_T new_value,
		Observer_T& observer,
		uint32_t time)
{
	bool all_satisfied = true;

	typename condition_list_t::iterator it = observer.condition_list.begin();
	for(; it != observer.condition_list.end(); ++it)
	{
		switch ( (*it).value_type)
		{
		case ConditionalObserveValueType::FLOAT:
			all_satisfied &= coap_satisfies_condition<Value_T, Observer_T, float>(new_value, observer, *it, time);
			break;
		default:
			all_satisfied &= coap_satisfies_condition<Value_T, Observer_T, uint32_t>(new_value, observer, *it, time);
			break;
		}

	}

	return all_satisfied;
}

coap_condition coap_parse_condition( OpaqueData raw )
{
	uint8_t *data = raw.value();
	coap_condition condition;

	uint8_t temp;
	uint32_t value = 0;

	temp = data[0];
	condition.type = (ConditionalObserveType::ConditionType) (temp >> 3);

	temp =  temp << 5;
	condition.reliability = (ConditionalObserveReliability::ObserveReliability) (temp >> 7);

	temp = temp << 1;
	condition.value_type = (ConditionalObserveValueType::ObserveValueType) (temp >> 6);

	for(size_t i = 1; i < raw.length(); i++)
	{
		value = value << 8;
		value = value | data[i];
	}

	condition.condition_value_raw = value;

	printf("COND_OBS: Generated condition: TYPE[%d] R[%d] V[%d] VAL[%d]\n",
			condition.type,
			condition.reliability,
			condition.value_type,
			condition.condition_value_raw);

	return condition;
}

template<typename T> inline const T abs(T const & x)
{
    return ( x<0 ) ? -x : x;
}

}

#endif /* COAP_CONDITIONAL_OBSERVE_H_ */
