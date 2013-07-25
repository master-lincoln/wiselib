/*
 * coap_high_level_states.h
 *
 *  Created on: Jul 23, 2013
 *      Author: wiselib
 */

#ifndef COAP_HIGH_LEVEL_STATES_H_
#define COAP_HIGH_LEVEL_STATES_H_

#include "util/pstl/vector_static.h"

namespace wiselib {

static const int COAP_MAX_HL_STATES = 10;
static const int COAP_MAX_STATE_RESOURCES = 10;

namespace HighLevelCreationType {
	enum CreationType {
		INTEGER,
		FLOAT,
		STRING
	};
}
typedef HighLevelCreationType::CreationType creation_type_t;

namespace HighLevelQueryType {
	enum QueryType {
		STATE,
		STATE_NUMBER,
		STATE_DESCRIPTION
	};
}
typedef HighLevelQueryType::QueryType query_type_t;


template<typename T, typename String>
struct number_state
{
	T lower_bound;
	T upper_bound;
	String name;

	String to_json();
};

template<typename String>
struct number_state<int, String>
{
	int lower_bound;
	int upper_bound;
	String name;

	String to_json();
};

template<typename Os_Model, typename T, typename String>
struct number_state_resource
{
	creation_type_t type;
	String path;
	vector_static<Os_Model, number_state<T, String>, COAP_MAX_HL_STATES> states;

	String to_json();
	String get_state(T sensor_value);
};



template<typename Os_Model, typename T, typename String>
String number_state_resource<Os_Model, T, String>::to_json()
{
	String result = String("");
	// TODO make sure String will not exceed 64 characters or use char*
	result.append("{p:'");
	result.append(path);
	result.append("', num:[");
	for (size_t i=0; i<states.size(); i++)
	{
		String json = states.at(i).to_json();
		result.append( json );
	}
	result.append("]}");

	return result;
}

template<typename String>
String number_state<int, String>::to_json() {
	String result = String("");

	char low_char[8];
	snprintf(low_char,8,"%d",lower_bound);
	String low = String(low_char);

	char up_char[8];
	snprintf(up_char,8,"%d",upper_bound);
	String high = String(up_char);

	result.append("{l:");
	result.append(low);
	result.append(",h:");
	result.append(high);
	result.append(",s:'");
	result.append(name);
	result.append("'}");

	return result;
}


}



#endif /* COAP_HIGH_LEVEL_STATES_H_ */
