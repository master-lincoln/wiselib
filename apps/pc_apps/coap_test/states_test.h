#ifndef STATES_TEST_H_
#define STATES_TEST_H_

#include "radio/coap/coap_service.h"
#include "radio/coap/coap_high_level_states.h"
#include "radio/coap/high_level_states_service.h"

#define STATES_TEST_INTERVAL 2000

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename String_T>
class StatesTest
{
public:

	typedef CoapRadio_P Radio;
	typedef Os_P Os;
	typedef String_T string_t;
	typedef uint16_t value_t;
	typedef typename Os::Debug Debug;
	typedef typename Os::Timer Timer;
	typedef StatesTest<Os, Radio, string_t> self_type;
	typedef CoapService<Os, Radio, string_t, value_t> coap_service_t;
	typedef HLStatesService<Os, Radio, string_t, value_t> hls_service_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;


	// --------------------------------------------------------------------------
	StatesTest() :
		num_(1)
	{	}
	// --------------------------------------------------------------------------
	void init(string_t path, Radio& radio, typename Os::Timer::self_pointer_t timer)
	{
		timer_ = timer;
		coap_service_.init("states", radio);
		coap_service_.set_handle_subresources(true);
		coap_service_.template set_request_callback<self_type, &self_type::handle_request>(this);

		hls_service_.init(coap_service_);
		hls_service_.template set_request_callback<coap_service_t, &coap_service_t::handle_request>(&coap_service_);
		hls_service_.register_at_radio();

		timer_->template set_timer<self_type, &self_type::gen_number>(STATES_TEST_INTERVAL, this, 0);
	}
	// --------------------------------------------------------------------------
	void gen_number(void*)
	{
		num_ = (++num_);// % 1000;
		coap_service_.set_status(num_);
		timer_->template set_timer<self_type, &self_type::gen_number>(STATES_TEST_INTERVAL, this, 0);
	}
	// --------------------------------------------------------------------------
	void handle_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();

		switch ( packet.code() ) {
			case COAP_CODE_GET:
				{
					char buffer[3];
					int val_len = sprintf( buffer, "%d", coap_service_.status() );
					coap_service_.radio()->reply( msg, (uint8_t *) buffer, val_len );
				}
				break;
			case COAP_CODE_POST:
				//cout << "POST Request\n";
				break;
			case COAP_CODE_PUT:
				//cout << "PUT Request\n";
				break;
			case COAP_CODE_DELETE:
				//cout << "DELETE Request\n";
				break;
			default:
				break;
		}
	}

private:
	typename Timer::self_pointer_t timer_;
	value_t num_;
	coap_service_t coap_service_;
	hls_service_t hls_service_;

};

}

#endif /* STATES_TEST_H_ */
