#ifndef OBSERVABLE_SERVICE_H_
#define OBSERVABLE_SERVICE_H_

#include "util/pstl/vector_static.h"

#include "radio/coap/coap_packet_static.h"
#include "radio/coap/coap_service_static.h"
#include "radio/coap/coap.h"

#include "service.h"

#define DEBUG_OBSERVE

#ifdef DEBUG_OBSERVE
#define DBG_OBS(...) debug_->debug( __VA_ARGS__)
#else
#define DBG_OBS(X)
#endif

namespace wiselib
{

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
	typedef ObservableService self_type;

	struct observer
	{
		node_id_t host_id;
		uint16_t last_mid;
		OpaqueData token;
		uint32_t timestamp;
	};

	struct message_data
	{
		block_data_t* data;
		size_t length;
	};

	typedef struct observer observer_t;
    typedef vector_static<Os, observer_t, COAP_MAX_OBSERVERS> observer_vector_t;
    typedef typename observer_vector_t::iterator observer_iterator_t;


	virtual ~ObservableService() { }

	ObservableService(string_t path, value_t initialStatus, Radio& radio)
	{
		path_ = path;
		status_ = initialStatus;
		radio_ = &radio;

		updateNotificationConfirmable_ = true;
		maxAge_ = COAP_DEFAULT_MAX_AGE;
		observe_counter_ = 1;
	}

	void init()
	{
		radio_->template reg_resource_callback<self_type, &self_type::receive_coap >( path_, this );
	}

	virtual void receive_coap(coap_message_t &msg) {

		coap_packet_t & packet = msg.message();
		uint32_t observe_value;

		if( packet.is_request() ){

			if ( packet.get_option(COAP_OPT_OBSERVE, observe_value) == coap_packet_t::SUCCESS ) {
				if ( add_observer(msg) )
				{
					coap_packet_t *sent = send_notification(observers_.back(), true);
					msg.set_ack_sent(sent);
				}
			}
			else
			{
				// no OBSERVE option -> remove correspondent from observers
				remove_observer(msg);
				// TODO call regular callback
			}

		} else {
			cout << "DA fuck !!!!!\n";
		}
	}

	string_t path()
	{
		return path_;
	}

	value_t status()
	{
		return status_;
	}

	void set_status(value_t newStatus)
	{
		status_ = newStatus;
		notify_observers();
		//timer_->template set_timer<self_type, &self_type::schedule_max_age_notifications>(maxAge_, this, 0);
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

	}

	void notify_observers() {
		observe_counter_++;
		for (observer_iterator_t it = observers_.begin(); it != observers_.end(); it++)
		{
			send_notification(*it);
		}
	}

	virtual void convert(value_t value, message_data& payload) = 0;

private:
	Radio *radio_;
	Timer *timer_;
	Debug *debug_;
	Clock *clock_;

	string_t path_;
	value_t status_;
	uint32_t maxAge_;
    bool updateNotificationConfirmable_;
    observer_vector_t observers_;
    uint32_t observe_counter_;


    void schedule_max_age_notifications( void*)
	{
    	notify_observers();
		timer_->template set_timer<self_type, &self_type::schedule_max_age_notifications>(maxAge_, this, 0);
	}

    /*!
       * @abstract Register a new observer, or update his token
       * @return  true if observer was added
       * @param   msg   CoAP message with observe request
       * @param   host_id  Node ID of observer
       */
      bool add_observer(coap_message_t& msg)
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
              for (observer_iterator_t it = observers_.begin(); it != observers_.end(); it++)
              {
                  if ( it->host_id == msg.correspondent() )
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
              observers_.push_back(new_observer);
              DBG_OBS("OBSERVE: Added host %x", new_observer.host_id);
              return true;
          }
      }

      void remove_observer(coap_message_t& msg) {
		  for (observer_iterator_t it = observers_.begin(); it != observers_.end(); it++) {
			  if ( it->host_id == msg.correspondent() )
			  {
				  DBG_OBS("OBSERVE: Removed host %x", it->host_id);
				  observers_.erase(it);
				  return;
			  }
		  }
	  }

      coap_packet_t* send_notification(observer_t& observer, bool first_notification = false)
      {
    	coap_packet_t answer;
		answer.set_option(COAP_OPT_OBSERVE, observe_counter_);
		answer.set_token(observer.token);
		answer.set_code( COAP_CODE_CONTENT );

		message_data payload;
		convert(status_, payload);
		answer.set_data( payload.data, payload.length );

		coap_packet_t *sent;

		if ( first_notification )
		{
			// send response piggy-backed with ACK
			answer.set_type(COAP_MSG_TYPE_ACK);
			answer.set_msg_id(observer.last_mid);
			sent = radio_->template send_coap_as_is<self_type, &self_type::got_ack>(observer.host_id, answer, this);
		}
		else
		{
			answer.set_type(COAP_MSG_TYPE_CON);
			sent = radio_->template send_coap_gen_msg_id<self_type, &self_type::got_ack>(observer.host_id, answer, this);
		}
		return sent;
      }

      void got_ack(coap_message_t& message) {
          DBG_OBS("OBSERVE: GOT ACK");
      }

      uint32_t time() {
    	  return clock_->seconds(clock_->time());
      };

};



}
#endif /* ObservableService_H_ */
