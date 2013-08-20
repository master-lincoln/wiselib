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

// --------------------------------------------------------------------------
namespace HighLevelCreationType {
	enum CreationType {
		INTEGER,
		FLOAT,
		STRING
	};
}
typedef HighLevelCreationType::CreationType creation_type_t;
// --------------------------------------------------------------------------
namespace HighLevelQueryType {
	enum QueryType {
		STATE,
		STATE_NUMBER,
		STATE_DESCRIPTION
	};
}
typedef HighLevelQueryType::QueryType query_type_t;
// --------------------------------------------------------------------------
template<typename T>
struct number_state
{
	T lower_bound;
	T upper_bound;
	const char* name;

	char* to_json();
	const char* oi();
};
// --------------------------------------------------------------------------
template<typename Os_Model, typename T>
struct number_state_resource
{
	creation_type_t type;
	const char* path;
	vector_static<Os_Model, number_state<T>, COAP_MAX_HL_STATES> states;

	char* to_json();
	char* get_state(T sensor_value);
};
// --------------------------------------------------------------------------
template<typename Os_Model, typename T>
char* number_state_resource<Os_Model, T>::to_json()
{
	uint16_t size = 32 * states.size() + 23 + 64;
	char* result = new char[size];
	char p[5] = "{p:'";
	char n[9] = "', num:[";
	char c[3] = "]}";

	strcat(result, p);
	strcat(result, path);
	strcat(result, n);

	for (size_t i=0; i<states.size(); i++)
	{
		char* json = states.at(i).to_json();
		strcat(result, json);
	}
	strcat(result, c);

	return result;
}
// --------------------------------------------------------------------------
template<>
char* number_state<int>::to_json() {
	char* result = new char[32];
	char l[4] = "{l:";
	char h[4] = ",h:";
	char s[5] = ",s:'";
	char c[3] = "'}";

	char low_char[8];
	snprintf(low_char,8,"%d",lower_bound);

	char up_char[8];
	snprintf(up_char,8,"%d",upper_bound);

	strcat(result, l);
	strcat(result, low_char);
	strcat(result, h);
	strcat(result, up_char);
	strcat(result, s);
	strcat(result, name);
	strcat(result, c);

	return result;
}
// --------------------------------------------------------------------------
template<>
char* number_state<float>::to_json() {
	char* result = new char[32];
	char l[4] = "{l:";
	char h[4] = ",h:";
	char s[5] = ",s:'";
	char c[3] = "'}";

	char low_char[8];
	snprintf(low_char,8,"%f",lower_bound);

	char up_char[8];
	snprintf(up_char,8,"%f",upper_bound);

	strcat(result, l);
	strcat(result, low_char);
	strcat(result, h);
	strcat(result, up_char);
	strcat(result, s);
	strcat(result, name);
	strcat(result, c);

	return result;
}
// --------------------------------------------------------------------------
template<>
const char* number_state<float>::oi() {
	const char* result = "oi!!!\n";
	cout << result;
	return result;
}


}



#endif /* COAP_HIGH_LEVEL_STATES_H_ */
