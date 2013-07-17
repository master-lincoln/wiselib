#ifndef CALCULATOR_H_
#define CALCULATOR_H_


#include <boost/algorithm/string.hpp>
#include "radio/coap/coap_packet_static.h"
#include "radio/coap/coap_service_static.h"

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename String_T>
class Calculator
{
public:
	typedef CoapRadio_P Radio;
	typedef Os_P Os;
	typedef String_T string_t;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef typename Radio::ReceivedMessage coap_message_t;

	Calculator& operator=( const Calculator &rhs )
	{
		// avoid self-assignment
		if(this != &rhs)
		{
			radio_ = rhs.radio_;
		}
		return *this;
	}

	Calculator()
	{

	}

	Calculator( const Calculator &rhs )
	{
		*this = rhs;
	}

	~Calculator()
	{

	}

	void init( Radio& radio)
	{
		radio_ = &radio;
	}

	void receive_coap( coap_message_t &message )
	{
		coap_packet_t & packet = message.message();
		cout << "Calculator::receive_coap> Received Code"
				<< (int) packet.code() << " with "
				<< packet.data_length() << " Bytes payload\n";
		typename Radio::block_data_t reply[100];
		size_t reply_length = 0;

		if( packet.is_request() )
		{
			wiselib::list_static<Os, string_t, 10> query;
			packet.get_query_list( COAP_OPT_URI_QUERY, query );
			int32_t received_num1 = 0;
			int32_t received_num2 = 0;
			char received_op = '+';

			typename wiselib::list_static<Os, string_t, 10>::iterator qit = query.begin();
			for( ; qit != query.end(); ++qit )
			{
				if( strstr( (*qit).c_str(), "num1=" ) != NULL )
				{
					received_num1 = (int32_t) atoi( (*qit).c_str() + 5 );
				}
				if( strstr( (*qit).c_str(), "num2=" ) != NULL )
				{
					received_num2 = (int32_t) atoi( (*qit).c_str() + 5 );
				}
				if( strstr( (*qit).c_str(), "op=" ) != NULL )
				{
					received_op = (*qit).c_str()[3];
				}
			}

			int32_t result = 0;
			switch( received_op )
			{
			case '+':
				result = received_num1 + received_num2;
				break;
			case '-':
				result = received_num1 - received_num2;
				break;
			case '/':
				// clearly division by zero results in 42!
				if(received_num2 == 0)
					result = 42;
				else
					result = received_num1 / received_num2;
				break;
			case '*':
				result = received_num1 * received_num2;
				break;
			default:
				break;
				// nop
			}

			cout << received_num1 << " " << (char) received_op
					<< " " << received_num2 << " = " << result <<"\n";

			reply_length = sprintf( (char*) reply, "%i %c %i = %i",
					received_num1, received_op, received_num2, result );

			cout << "sending " << reply_length << " bytes payload in reply: ";
			for(size_t m = 0; m < reply_length; ++m )
			{
				cout << (int) reply[m] << " ";
			}

			radio_->reply( message, reply, reply_length );

		}

	}

private:
	Radio *radio_;
};

}
#endif /* CALCULATOR_H_ */
