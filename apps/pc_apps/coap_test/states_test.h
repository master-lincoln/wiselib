#ifndef STATES_TEST_H_
#define STATES_TEST_H_

#include "radio/coap/coap_service.h"
#include "radio/coap/coap_high_level_states.h"

#define STATES_TEST_INTERVAL 10000

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename String_T>
class StatesTest : public CoapService<Os_P, CoapRadio_P, String_T>
{
public:

	typedef CoapRadio_P Radio;
	typedef Os_P Os;
	typedef String_T string_t;
	typedef typename Os::Debug Debug;
	typedef typename Os::Timer Timer;
	typedef StatesTest<Os, Radio, string_t> self_type;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;

	typedef vector_static<Os, number_state_resource<Os, int, string_t>, COAP_MAX_STATE_RESOURCES> state_resources_vector_t;

	// --------------------------------------------------------------------------
	StatesTest(string_t path, Radio& radio, typename Os::Timer* timer) :
		CoapService<Os_P, CoapRadio_P, String_T>(path, radio),
		num_(1),
		timer_(timer)
	{
		this->template set_request_callback<self_type, &self_type::handle_request>(this);
		this->set_handle_subresources(true);
		timer_->template set_timer<self_type, &self_type::gen_number>(STATES_TEST_INTERVAL, this, 0);

		// ----------------------------------------------------
		number_state_resource<Os, int, string_t> hls_res;
		hls_res.type = HighLevelCreationType::INTEGER;
		hls_res.path = string_t("mr21");

		number_state<int, string_t> state1 = {0,20,"low"};
		number_state<int, string_t> state2 = {20,40,"high"};

		hls_res.states.push_back(state1);
		hls_res.states.push_back(state2);
		state_resources_.push_back(hls_res);

		int radio_reg_id_ = this->radio()->template reg_resource_callback<self_type, &self_type::dummy >( this->path().append("/mr21"), this );
		// ----------------------------------------------------
	}

	void dummy(coap_message_t& msg) {}
	// --------------------------------------------------------------------------
	void gen_number(void*)
	{
		num_ = (++num_) % 1000;
		timer_->template set_timer<self_type, &self_type::gen_number>(STATES_TEST_INTERVAL, this, 0);
	}
	// --------------------------------------------------------------------------
	void handle_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();

		switch ( packet.code() ) {
			case COAP_CODE_GET:
				handle_get_request(msg);
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
	// --------------------------------------------------------------------------
	void handle_get_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();
		OpaqueData hl_data;

		if ( packet.get_option(COAP_OPT_HL_STATE, hl_data) == coap_packet_t::SUCCESS )
		{
			uint8_t type;
			size_t length;
			hl_data.get(&type, length);

			switch (type) {
				case HighLevelQueryType::STATE:
					cout << "Aksing for State\n";
					break;
				case HighLevelQueryType::STATE_NUMBER:
					cout << "Aksing for Number\n";
					break;
				case HighLevelQueryType::STATE_DESCRIPTION:
					cout << "Aksing for Description\n";
					if ( packet.uri_path() == this->path() )
					{
						char* json = resources_to_json();
						cout << "Message: " << json << "\n";
						this->radio()->reply( msg, (uint8_t*) json, strlen(json) );
					}
					break;
				default:
					// Query Type not supported -> send Bad_Option
					this->radio()->reply( msg, (uint8_t*) 0, 0, COAP_CODE_BAD_OPTION );
					break;
			}

		}
	}


	char* resources_to_json()
	{
		string_t delim = ",";
		string_t res = "{res:{r:[";
		string_t footer = "]}}";
		char* buffer = new char[1024];

		strcpy(buffer, res.c_str());

		for ( size_t i=0; i<state_resources_.size(); i++)
		{
			string_t json = string_t(state_resources_.at(i).to_json());
			strcat(buffer, json.c_str());

			if ( i < (state_resources_.size()-1) )
			{
				strcat(buffer, delim.c_str());
			}
		}
		strcat(buffer, footer.c_str());

		return buffer;
	}

private:
	Timer *timer_;
	int num_;

	state_resources_vector_t state_resources_;

};

}

#endif /* STATES_TEST_H_ */
