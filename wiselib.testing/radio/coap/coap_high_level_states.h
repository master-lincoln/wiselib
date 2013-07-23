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

enum HighLevelCreationType {
	INTEGER,
	FLOAT,
	STRING
};

enum HighLevelQueryType {
	STATE,
	STATE_NUMBER,
	STATE_DESCRIPTION
};

template<typename Os_Model, typename T, typename String>
struct number_state_resource {
	HighLevelCreationType type;
	String path;
	vector_static<Os_Model, number_state, COAP_MAX_HL_STATES> states;

	String to_json() const;
};

template<typename T, typename String>
struct number_state {
	T lower_bound;
	T upper_bound;
	String name;

	String to_json() const;
};

template<typename Os_Model, typename T, typename String>
String number_state_resource::to_json() const {
	String result = String("");

	result.append("{p:'");
	result.append(path);
	result.append("', num:[");
	// TODO for each number state -> to_json
	result.append("]}");

	return result;
}

template<typename T, typename String>
String number_state::to_json() const {
	String result = String("");
	// TODO high/low to String
	String low = String("0");
	String high = String("10");

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
