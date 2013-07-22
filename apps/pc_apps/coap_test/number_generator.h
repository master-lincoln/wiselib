#ifndef NUMBER_GENERATOR_H_
#define NUMBER_GENERATOR_H_


#include "radio/coap/coap_packet_static.h"
#include "radio/coap/coap_service_static.h"

#define SELF_TYPE NumberGenerator<Os, Radio, Timer, string_t>

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename Timer_P, typename String_T>
class NumberGenerator
{
public:
	typedef CoapRadio_P Radio;
	typedef Os_P Os;
	typedef String_T string_t;
	typedef typename Os::Rand Rand;
	typedef typename Os::Debug Debug;
	typedef Timer_P Timer;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::node_id_t node_id_t;
	typedef SELF_TYPE self_type;

	NumberGenerator& operator=( const NumberGenerator &rhs )
	{
		// avoid self-assignment
		if(this != &rhs)
		{
			radio_ = rhs.radio_;
		}
		return *this;
	}

	NumberGenerator( const NumberGenerator &rhs )
	{
		*this = rhs;
	}

	NumberGenerator() { }
	~NumberGenerator() { }

	void init( Radio& radio)
	{
		radio_ = &radio;
		rand_->srand( radio_->id() );
		num_ = 0;
		observed_ = false;
		timer_->template set_timer<self_type, &self_type::generate_number>(1000, this, 0);
	}

	void generate_number(void*) {
		//num_ = rand_->operator()(100);
		num_ = (num_ + 1) % (1000);
		if (observed_) {
			send_number_notify(false);
		}
		timer_->template set_timer<self_type, &self_type::generate_number>(10000, this, 0);
	}

	void send_number_notify(bool first) {
		char buffer[3];
		int len = sprintf(buffer, "%d", num_);
		notify((block_data_t*)buffer, len, first);
		string_t first_s = first ? " first" : "";
		cout << "Send" << first_s.c_str() << " Notification " << num_ << "\n";
	}

	void notify(block_data_t *data, size_t length, bool first) {
		coap_packet_t answer;
		OpaqueData token;

		answer.set_option(COAP_OPT_OBSERVE, num_);

		observe_packet_->message().token(token);
		answer.set_token(token);

		answer.set_type(COAP_MSG_TYPE_CON);

		answer.set_code( COAP_CODE_CONTENT );
		answer.set_data( data, length );

		node_id_t to = observe_packet_->correspondent();

		if ( first )
		{
			answer.set_type(COAP_MSG_TYPE_ACK);
			answer.set_msg_id(observe_packet_->message().msg_id());
			coap_packet_t *sent = radio_->template send_coap_as_is<self_type, &self_type::got_ack>(to, answer, this);
			observe_packet_->set_ack_sent(sent);
		}
		else
		{
			// TODO handle not getting ACK
			coap_packet_t *sent = radio_->template send_coap_gen_msg_id<self_type, &self_type::got_ack>(to, answer, this);
		}

		//cout << "Send notification to: " << to << " with ID "<< sent->msg_id() << "\n";

		//radio_->reply(*observe_packet_, data, length, COAP_CODE_CONTENT, answer);
	}

	void got_ack(coap_message_t& msg) {
		DBG_COAP("Got ACK");
	}

	void receive_coap( coap_message_t &message )
	{
		coap_packet_t & packet = message.message();

		//print_packet(packet);

		if( packet.is_request() ){

			uint32_t observe_value;
			if ( packet.get_option(COAP_OPT_OBSERVE, observe_value) == coap_packet_t::SUCCESS) {
				cout << "GOT AN OBSERVE REQUEST:" << observe_value << "\n";
				observed_ = true;
				observe_packet_ = &message;
				send_number_notify(true);
				return;
			} else if (observed_) {
				// GET request without observe cancels notifications
				DBG_COAP("Canceled Observe");
				observed_ = false;
			}

			char buffer[3];
			int len = sprintf(buffer, "%d", num_);

			radio_->reply( message, (uint8_t *) buffer, len );

		}

	}

private:
	Radio *radio_;
	Rand *rand_;
	Timer *timer_;
	Debug *debug_;
	int num_;
	bool observed_;
	coap_message_t *observe_packet_;
	coap_message_t *last_notification;
};

}
#endif /* NumberGenerator_H_ */
