#ifndef OBS_TEST_H_
#define OBS_TEST_H_

#include "radio/coap/observable_service.h"

#define OBS_TEST_INTERVAL 4000

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename String_T>
class ObsTest
{
public:

	typedef CoapRadio_P Radio;
	typedef Os_P Os;
	typedef String_T string_t;
	typedef typename Os::Debug Debug;
	typedef typename Os::Timer Timer;
	typedef ObsTest<Os, Radio, string_t> self_type;
	typedef ObservableService<Os_P, CoapRadio_P, String_T, uint16_t> coap_obs_t;
	typedef CoapService<Os_P, CoapRadio_P, String_T, uint16_t> coap_service_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef struct coap_obs_t::message_data message_data;

	// --------------------------------------------------------------------------
	ObsTest(string_t path, Radio& radio) :
		num_(100)
	{
		coap_service_->init("observe", radio);
		coap_service_->set_handle_subresources(false);
		coap_service_->template set_request_callback<self_type, &self_type::handle_request>(this);

		coap_observable_ = new coap_obs_t(path, *coap_service_);
		coap_observable_->set_max_age(20);
		coap_observable_->set_update_notification_confirmable(false);
		coap_observable_->template set_request_callback<coap_service_t, &coap_service_t::handle_request>(coap_service_);
		coap_observable_->register_at_radio();

		// generate fake data
		timer_->template set_timer<self_type, &self_type::gen_number>(OBS_TEST_INTERVAL, this, 0);
	}
	// --------------------------------------------------------------------------
	void gen_number(void*)
	{
		num_ = (++num_) % 1000;
		coap_service_->set_status(num_);
		timer_->template set_timer<self_type, &self_type::gen_number>(OBS_TEST_INTERVAL, this, 0);
	}
	// --------------------------------------------------------------------------
	void handle_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();

		switch ( packet.code() ) {
			case COAP_CODE_GET:
				{
					char buffer[3];
					int val_len = sprintf( buffer, "%d", coap_service_->status() );

					coap_service_->radio()->reply( msg, (uint8_t *) buffer, val_len );
					break;
				}
			case COAP_CODE_POST:
				debug_->debug("POST Request");
				break;
			case COAP_CODE_PUT:
				debug_->debug("PUT Request");
				break;
			case COAP_CODE_DELETE:
				debug_->debug("DELETE Request");
				break;
			default:
				break;
		}
	}

private:
	Timer *timer_;
	Debug *debug_;
	uint16_t num_;
	coap_service_t *coap_service_;
	coap_obs_t *coap_observable_;

};

}

#endif /* OBS_TEST_H_ */
