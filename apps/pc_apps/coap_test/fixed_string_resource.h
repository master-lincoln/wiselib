#ifndef FIXEDSTRINGRESOURCE_H_
#define FIXEDSTRINGRESOURCE_H_

namespace wiselib
{

template<typename CoapRadio_P>
class FixedStringResource
{
public:
	typedef CoapRadio_P Radio;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef FixedStringResource<Radio> self_t;

	FixedStringResource& operator=( const FixedStringResource &rhs )
	{
		// avoid self-assignment
		if(this != &rhs)
		{
			radio_ = rhs.radio_;
		}
		return *this;
	}

	FixedStringResource( Radio& radio, StaticString path, StaticString text )
	{
		radio_ = &radio;
		text_ = text;

		// register callback
		radio_-> template reg_resource_callback< self_t, &self_t::receive_coap >( path, this );
	}

	FixedStringResource( const FixedStringResource &rhs )
	{
		*this = rhs;
	}

	~FixedStringResource()
	{

	}

	void receive_coap( coap_message_t &message )
	{
		radio_->reply( message, (block_data_t*) text_.c_str(), text_.size() );
	}

private:
	Radio *radio_;
	StaticString text_;
};

}
#endif /* FIXEDSTRINGRESOURCE_H_ */
