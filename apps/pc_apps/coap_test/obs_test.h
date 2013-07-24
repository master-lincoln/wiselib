#ifndef OBS_TEST_H_
#define OBS_TEST_H_

#include "radio/coap/observable_service.h"

#define OBS_TEST_INTERVAL 40000

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
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef struct ObservableService<Os_P, CoapRadio_P, String_T, uint>::message_data message_data;
	// --------------------------------------------------------------------------
	ObsTest(string_t path, Radio& radio) :
		ObservableService<Os_P, CoapRadio_P, String_T, uint>(path, 0, radio)
	{
		num_ = 100;
		this->template set_request_callback<self_type, &self_type::handle_request>(this);
		this->set_max_age(20);
		this->set_update_notification_confirmable(false);
		this->set_handle_subresources(false);
		timer_->template set_timer<self_type, &self_type::gen_number>(OBS_TEST_INTERVAL, this, 0);
	}
	// --------------------------------------------------------------------------
	void gen_number(void*)
	{
		num_ = (++num_) % 1000;
		this->set_status(num_);
		timer_->template set_timer<self_type, &self_type::gen_number>(OBS_TEST_INTERVAL, this, 0);
	}
	// --------------------------------------------------------------------------
	void convert(uint value, message_data& payload)
	{
		char data[10];
		payload.length = sprintf(data, "%d", value);
		payload.data = (block_data_t*) data;
	}
	// --------------------------------------------------------------------------
	void handle_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();

		switch ( packet.code() ) {
			case COAP_CODE_GET:
				cout << "GET Request\n";
				break;
			case COAP_CODE_POST:
				cout << "POST Request\n";
				break;
			case COAP_CODE_PUT:
				cout << "PUT Request\n";
				break;
			case COAP_CODE_DELETE:
				cout << "DELETE Request\n";
				break;
			default:
				break;
		}
	}

private:
	Timer *timer_;
	Debug *debug_;
	uint16_t num_;

};

}

#endif /* OBS_TEST_H_ */
