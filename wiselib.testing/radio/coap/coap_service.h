#ifndef COAP_SERVICE_H_
#define COAP_SERVICE_H_

#include "util.h"

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename String_T>
class CoapService
{
public:

	typedef Os_P Os;
	typedef String_T string_t;
	typedef CoapRadio_P Radio;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef delegate1<void, coap_message_t&> coapreceiver_delegate_t;
	typedef CoapService<Os, Radio, string_t> self_type;

	~CoapService() { }

	CoapService(string_t path, Radio& radio) :
		path_(path),
		radio_(&radio),
		handle_subresources_(false),
		request_callback_(coapreceiver_delegate_t()),
		radio_reg_id_(0)
	{
		radio_reg_id_ = radio_->template reg_resource_callback<self_type, &self_type::receive_coap >( path_, this );
	}

	void receive_coap(coap_message_t &msg) {

		coap_packet_t & packet = msg.message();

		int path_compare = path_cmp<string_t>( path_, packet.uri_path() );
		if( (handle_subresources_ || path_compare == EQUAL) && request_callback_ && request_callback_.obj_ptr() != NULL )
		{
			request_callback_(msg);
		}
	}

	inline string_t path()
	{
		return path_;
	}

	inline Radio* radio()
	{
		return radio_;
	}

	inline bool handles_subresources()
	{
		return handle_subresources_;
	}

	void set_handle_subresources(bool handle_subresources)
	{
		handle_subresources_ = handle_subresources;
	}

	void shutdown()
	{
		radio_->unreg_resource_callback(radio_reg_id_);
	}

	template <class T, void (T::*TMethod)( typename self_type::coap_message_t & ) >
	void set_request_callback( T *callback )
	{
		request_callback_ = coapreceiver_delegate_t::template from_method<T, TMethod>( callback );
	}

private:
	Radio *radio_;
	string_t path_;
	int radio_reg_id_;
	bool handle_subresources_;						// if set, parent also receives all subresource requests
	coapreceiver_delegate_t request_callback_;

};

}

#endif /* COAP_SERVICE_H_ */
