/*
 * obs_test.h
 *
 *  Created on: Jul 15, 2013
 *      Author: wiselib
 */

#ifndef OBS_TEST_H_
#define OBS_TEST_H_

#include "observable_service.h"

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename String_T>
class ObsTest : public ObservableService<Os_P, CoapRadio_P, String_T, uint>
{
public:

	typedef CoapRadio_P Radio;
	typedef Os_P Os;
	typedef String_T string_t;
	typedef typename Os::Debug Debug;
	typedef typename Os::Timer Timer;
	typedef ObsTest<Os, Radio, string_t> self_type;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef struct ObservableService<Os_P, CoapRadio_P, String_T, uint>::message_data message_data;

	ObsTest(string_t path, Radio& radio) :
		ObservableService<Os_P, CoapRadio_P, String_T, uint>(path, 0, radio)
	{
		this->init();
		this->template set_request_callback<self_type, &self_type::handle_request>(this);
		timer_->template set_timer<self_type, &self_type::gen_number>(5000, this, 0);
	}

	void gen_number(void*) {
		num_ = (++num_) % 1000;
		this->set_status(num_);
		timer_->template set_timer<self_type, &self_type::gen_number>(5000, this, 0);
	}

	void convert(uint value, message_data& payload) {
		char data[10];
		payload.length = sprintf(data, "%d", value);
		payload.data = (block_data_t*) data;
	}

	void handle_request(coap_message_t& msg)
	{
		cout << "REQUEST\n";
	}

private:
	Timer *timer_;
	Debug *debug_;
	uint16_t num_;

};

}

#endif /* OBS_TEST_H_ */
