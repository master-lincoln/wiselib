#ifndef OBSERVABLE_SERVICE_H_
#define OBSERVABLE_SERVICE_H_

#include "util/pstl/vector_static.h"

#include "coap_service.h"
#include "coap_packet_static.h"
#include "coap_service_static.h"
#include "coap_conditional_observe.h"
#include "coap.h"

#define DEBUG_OBSERVE

#ifdef DEBUG_OBSERVE
#define DBG_OBS(...) debug_->debug( __VA_ARGS__)
#else
#define DBG_OBS(X)
#endif

namespace wiselib
{
template<typename Value_T>
struct observer<Value_T>
{
	node_id_t host_id;
	uint16_t last_mid;
	OpaqueData token;
	uint32_t timestamp;
	Value_T last_value;
	coap_condition* condition;
};

template<typename Os_P, typename CoapRadio_P, typename String_T, typename Value_T>
class ObservableService
{
public:

	typedef Os_P Os;
	typedef String_T string_t;
	typedef Value_T value_t;
	typedef CoapRadio_P Radio;
	typedef typename Os::Debug Debug;
	typedef typename Os::Timer Timer;
	typedef typename Os::Clock Clock;
	typedef typename Os::Clock::time_t time_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef delegate1<void, coap_message_t&> coapreceiver_delegate_t;
	typedef ObservableService self_type;
	typedef CoapService<Os, Radio, string_t, value_t> coap_service_t;

	struct message_data
	{
		block_data_t* data;
		size_t length;
	};

	typedef struct observer<value_t> observer_t;
    typedef vector_static<Os, observer_t, COAP_MAX_OBSERVERS> observer_vector_t;
    typedef typename observer_vector_t::iterator observer_iterator_t;


	virtual ~ObservableService() { }

	ObservableService(string_t path, coap_service_t& service):
		updateNotificationConfirmable_(true),
		maxAge_(COAP_DEFAULT_MAX_AGE),
		service_(&service),
		observe_counter_ (1),
		max_age_notifications_(0),
		radio_reg_id_(-1),
		request_callback_(coapreceiver_delegate_t()),
		status_(service.status())
	{
		service_->template add_status_listener<self_type, &self_type::set_status >( this );
	}

	void register_at_radio()
	{
		radio_reg_id_ = service_->radio()->template reg_resource_callback<self_type, &self_type::handle_request >( service_->path(), this );
	}

	void handle_request(coap_message_t &msg)
	{

		coap_packet_t & packet = msg.message();
		uint32_t observe_value;
		OpaqueData condition_raw;
		coap_condition* condition;


		if ( packet.get_option(COAP_OPT_OBSERVE, observe_value) == coap_packet_t::SUCCESS ) {
			// we got an observe request, add new observer or update token respectively
			// and send ACK with piggybacked sensor value
			if ( packet.get_option(COAP_OPT_CONDITION, condition_raw) == coap_packet_t::SUCCESS )
			{
				condition = & coap_parse_condition(condition_raw);
			}

			if ( add_observer(msg, condition) )
			{
				coap_packet_t *sent = send_notification(observers_.back(), true);
				msg.set_ack_sent(sent);
			}

		}
		else
		{
			// no OBSERVE option -> remove correspondent from observers
			remove_observer(msg);
			if ( request_callback_ && request_callback_.obj_ptr() != NULL )
			{
				request_callback_(msg);
			}

		}
	}

	uint32_t max_age()
	{
		return maxAge_;
	}

	void set_max_age(uint32_t maxAge)
	{
		maxAge_ = maxAge;
	}

	bool is_update_notification_confirmable()
	{
		return updateNotificationConfirmable_;
	}

	void set_update_notification_confirmable(bool updateNotificationConfirmable)
	{
		updateNotificationConfirmable_ = updateNotificationConfirmable;
	}

	CoapType message_type_for_notification()
	{
		return updateNotificationConfirmable_ ? COAP_MSG_TYPE_CON : COAP_MSG_TYPE_NON;
	}

	void shutdown()
	{
		// if there's a radio callback registered: unregister it
		if ( radio_reg_id_ != -1 )
			service_->radio().unreg_resource_callback( radio_reg_id_ );
	}

	void notify_observers() {
		observe_counter_++;
		max_age_notifications_++;
		for (observer_iterator_t it = observers_.begin(); it != observers_.end(); it++)
		{
			if ( it->condition == NULL ||
					coap_satisfies_condition( status_, it->condition, time() )
				)
			{
				send_notification(*it);
			}

		}
	}

	template <class T, void (T::*TMethod)( typename self_type::coap_message_t & ) >
	void set_request_callback( T *callback )
	{
		request_callback_ = coapreceiver_delegate_t::template from_method<T, TMethod>( callback );
	}

private:
	/** timer to schedule max_age notifications */
	Timer *timer_;
	Debug *debug_;
	Clock *clock_;
	coap_service_t *service_;

	/** cached status of the resource */
	value_t status_;
	/** max_age value specifying when to resend */
	uint32_t maxAge_;
	/** boolean indicating if acknowledgable messages should be used for observe notifications  */
	bool updateNotificationConfirmable_;
	/** vector of observing hosts including last message id and token */
	observer_vector_t observers_;
	// TODO what if this overflows?
	/** unique observe id */
	uint32_t observe_counter_;
	/** internal helper variable to make sure to only resend if the last max-age expired */
	uint8_t max_age_notifications_;
	coapreceiver_delegate_t request_callback_;
	int radio_reg_id_;

	void schedule_max_age_notifications(void*)
	{
		max_age_notifications_--;
		// only send if the last max-age expired
		if (max_age_notifications_ == 0 && observers_.size() > 0)
		{
			DBG_OBS("OBSERVE: Max-Age reached. Resending notifications...");
			notify_observers();
			timer_->template set_timer<self_type,
					&self_type::schedule_max_age_notifications>(maxAge_ * 1000,
					this, 0);
		}
	}

	/*!
	 * @abstract Register a new observer, or update his token
	 * @return  true if observer was added
	 * @param   msg   CoAP message with observe request
	 * @param   host_id  Node ID of observer
	 */
	bool add_observer(coap_message_t& msg, coap_condition* condition)
	{
		coap_packet_t packet = msg.message();
		OpaqueData token;
		packet.token(token);

		if (observers_.max_size() == observers_.size())
		{
			DBG_OBS("OBSERVE: MAX OBSERVERS REACHED. WON'T ADD");
			return false;
		}
		else
		{
			for (observer_iterator_t it = observers_.begin();
					it != observers_.end(); it++)
			{
				if (it->host_id == msg.correspondent())
				{
					//update token
					it->token = token;
					DBG_OBS("OBSERVE: TOKEN UPDATED");
					return true;
				}
			}
			observer_t new_observer;
			new_observer.host_id = msg.correspondent();
			new_observer.token = token;
			new_observer.last_mid = packet.msg_id();
			new_observer.timestamp = time();
			new_observer.last_value = status_;
			new_observer.condition = condition;
			observers_.push_back(new_observer);

			DBG_OBS("OBSERVE: Added host %x", new_observer.host_id);
			return true;
		}
	}

	void remove_observer(coap_message_t& msg)
	{
		for (observer_iterator_t it = observers_.begin();
				it != observers_.end(); it++)
		{
			if (it->host_id == msg.correspondent())
			{
				DBG_OBS("OBSERVE: Removed host %x", it->host_id);
				observers_.erase(it);
				return;
			}
		}
	}

	void set_status(value_t new_status)
	{
		status_ = new_status;
		notify_observers();
	}

	coap_packet_t* send_notification(observer_t& observer,
			bool first_notification = false)
	{
		coap_packet_t answer;
		answer.set_option(COAP_OPT_OBSERVE, observe_counter_);
		answer.set_token(observer.token);
		answer.set_code(COAP_CODE_CONTENT);

		message_data payload;
		convert(status_, payload);
		answer.set_data(payload.data, payload.length);

		coap_packet_t *sent;

		if (first_notification)
		{
			// send response piggy-backed with ACK
			answer.set_type(COAP_MSG_TYPE_ACK);
			answer.set_msg_id(observer.last_mid);
			sent = service_->radio()->template send_coap_as_is<self_type,
					&self_type::got_ack>(observer.host_id, answer, this);
		}
		else
		{
			answer.set_type(message_type_for_notification());
			sent = service_->radio()->template send_coap_gen_msg_id<self_type,
					&self_type::got_ack>(observer.host_id, answer, this);
		}

		if ( sent != NULL )
		{
			observer.last_mid = sent->msg_id();
			observer.timestamp = time();
			observer.last_value = status_;
		}
		return sent;
	}

	void got_ack(coap_message_t& message)
	{
		// TODO why is this never getting called
		DBG_OBS("OBSERVE: GOT ACK");
	}

	uint32_t time()
	{
		// TODO check if this makes sense
		return clock_->seconds(clock_->time());
	}

	template<typename V>
	void convert(V value, message_data& payload)
	{
		conv<V>(value, payload);
	}

	template<typename V>
	void conv(uint16_t value, message_data& payload)
	{
		char data[10];
		payload.length = sprintf(data, "%d", value);
		payload.data = (block_data_t*) data;
	}

	template<typename V>
	void conv(float value, message_data& payload)
	{
		char data[10];
		payload.length = sprintf(data, "%f", value);
		payload.data = (block_data_t*) data;
	}

	template<typename V>
	void conv(string_t value, message_data& payload)
	{
		payload.length = value.length();
		payload.data = (block_data_t*) value.c_str();
	}

};


}
#endif /* ObservableService_H_ */
