#ifndef COAPRADIO_H
#define COAPRADIO_H

#include "coap.h"
#include "coap_packet.h"

namespace wiselib {

template<typename OsModel_P,
	typename Radio_P,
	typename Timer_P,
	typename Debug_P>
	class CoapRadio
	: public RadioBase<OsModel_P,
		typename Radio_P::node_id_t,
		typename Radio_P::size_t,
		typename Radio_P::block_data_t> {


	struct SentMessage
	{
		CoapPacket<OsModel_P> *message_;
		uint8_t retransmit_count_;
	};

	struct ReceivedMessage
	{
		CoapPacket<OsModel_P> *message_;
		bool ack_;
		bool response_;
	};

	struct coapreceiver
	{
		// TODO:
		// Typ des Receivers (bekommt der nur die Daten, oder das ganze Paket?)
		// Callback Pointer oder sowas
		// string der die Resource bezeichnet
	};

	public:
		// Type definitions
		typedef OsModel_P OsModel;

		typedef Radio_P Radio;

		typedef Debug_P Debug;
		typedef Timer_P Timer;

		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::self_pointer_t self_pointer_t;
		
		typedef CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;

		void init( Radio& radio, Timer& timer, Debug& debug );
		int enable_radio();
		int disable_radio();
		node_id_t id ();
		int send (node_id_t receiver, size_t len, block_data_t *data );
		void receive(node_id_t from, size_t len, block_data_t * data);
		
		enum error_codes
		{
			// inherited from concepts::BasicReturnValues_concept
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL
		};

	private:
		Radio *radio_;
		Timer *timer_;
		Debug *debug_;
		int recv_callback_id_; // callback for receive function
		list_static<OsModel, CoapPacket<OsModel>, COAPRADIO_LIST_SIZE> message_buffer;

		uint32_t msg_id_;
		COAP_TOKEN_TYPE token_;

		uint32_t msg_id();
		COAP_TOKEN_TYPE token();

	};


// public
	template<typename OsModel_P,
			typename Radio_P,
			typename Timer_P,
			typename Debug_P>
	void CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P>::init(Radio& radio, Timer& timer, Debug& debug)
	{
		radio_ = &radio;
		timer_ = &timer;
		debug_ = &debug;

		// random initial message ID and token
		msg_id_ = rand();
		token_ = rand();

	}

	template<typename OsModel_P,
			typename Radio_P,
			typename Timer_P,
			typename Debug_P>
	int CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P>::enable_radio()
	{
#ifdef DEBUG_COAPRADIO
		debug_->debug("CoapRadio::enable\n");
#endif
		//enable normal radio
		radio_->enable_radio();
		// register receive callback to normal radio
		recv_callback_id_ = radio_->template reg_recv_callback<self_t,
			&self_t::receive > ( this);

		return SUCCESS;
	}

	template<typename OsModel_P,
			typename Radio_P,
			typename Timer_P,
			typename Debug_P>
	int CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P>::disable_radio()
	{
		radio_->unreg_recv_callback(recv_callback_id_);
		return SUCCESS;
	}

	template<typename OsModel_P,
			typename Radio_P,
			typename Timer_P,
			typename Debug_P>
	typename Radio_P::node_id_t CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P>::id ()
	{
		return radio_->id();
	}

	template<typename OsModel_P,
			typename Radio_P,
			typename Timer_P,
			typename Debug_P>
	void CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P>::receive(node_id_t from, size_t len, block_data_t * data)
	{
		// do not receive own messages
		if (radio_->id() == from) {
#ifdef DEBUG_COAPRADIO
			debug_->debug( "own message received\n");
#endif
			return;
		}
		if (len > 0 )
		{
			message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );
			if( msg_id == CoapMsgId )
			{
#ifdef DEBUG_COAPRADIO
				debug_->debug( "received coap message!\n");
#endif

			}
		}
	}


// private
	template<typename OsModel_P,
			typename Radio_P,
			typename Timer_P,
			typename Debug_P>
	uint32_t CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P>::msg_id()
	{
		return(msg_id_++);
	}

	template<typename OsModel_P,
			typename Radio_P,
			typename Timer_P,
			typename Debug_P>
	COAP_TOKEN_TYPE CoapRadio<OsModel_P, Radio_P, Timer_P, Debug_P>::token()
	{
		return(token_++);
	}

}


#endif // COAPRADIO_H