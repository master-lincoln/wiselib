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
	typedef typename Os::Rand Rand;
	typedef StatesTest<Os, Radio, string_t> self_type;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;

	typedef int value_t;
	typedef number_state_resource<Os, value_t> state_resource_t;
	typedef vector_static<Os, state_resource_t, COAP_MAX_STATE_RESOURCES> state_resources_vector_t;

	// --------------------------------------------------------------------------
	StatesTest(string_t path, Radio& radio, typename Os::Timer* timer) :
		CoapService<Os_P, CoapRadio_P, String_T>(path, radio),
		num_(1),
		status_(num_),
		timer_(timer)
	{
		this->template set_request_callback<self_type, &self_type::handle_request>(this);
		this->set_handle_subresources(true);
		timer_->template set_timer<self_type, &self_type::gen_number>(STATES_TEST_INTERVAL, this, 0);

		rand_->srand( radio.id() );

		// ----------------------------------------------------
		number_state_resource<Os, value_t> hls_res;
		hls_res.type = HighLevelCreationType::INTEGER;
		hls_res.path = "mr21";

		number_state<value_t> state1 = {0,20,"low"};
		number_state<value_t> state2 = {20,40,"high"};

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
		num_ = (++num_);// % 1000;
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
				handle_post_request(msg);
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
					char* json;
					if ( packet.uri_path() == this->path() )
					{
						json = resources_to_json();
					}
					else
					{
						state_resource_t* resource = find_state_resource( packet.uri_path() );
						if ( resource == NULL )
						{
							// no such resource found
							char * error_description = NULL;
							int len = 0;
							char error_description_str[COAP_ERROR_STRING_LEN];
							len = sprintf(error_description_str, "Resource \"%s\" not found.", packet.uri_path().c_str() );
							error_description = error_description_str;
							CoapContentType ctype = COAP_CONTENT_TYPE_TEXT_PLAIN;
							this->radio()->reply( msg, (uint8_t*) error_description, len, COAP_CODE_NOT_FOUND, ctype );

							return;
						}
						else
						{
							json = resource->to_json();
						}

					}
					cout << "Message: " << json << "\n";
					// TODO Content Type
					this->radio()->reply( msg, (uint8_t*) json, strlen(json) );

					break;
				default:
					// Query Type not supported -> send Bad_Option
					this->radio()->reply( msg, (uint8_t*) 0, 0, COAP_CODE_BAD_OPTION );
					break;
			}
		}
	}

	// --------------------------------------------------------------------------
	/**
	 * \brief Handles POST requests with High-Level-States options. Registers a new sub-resource
	 *  under a random 5 char URI.
	 */
	void handle_post_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();
		list_static<Os, OpaqueData, COAP_MAX_HL_STATES> hl_list;

		if ( packet.template get_options< list_static<Os, OpaqueData, COAP_MAX_HL_STATES> > (COAP_OPT_HL_STATE, hl_list) == coap_packet_t::SUCCESS )
		{

			typename list_static<Os, OpaqueData, COAP_MAX_HL_STATES>::iterator it = hl_list.begin();

			number_state_resource<Os, int> hls_res;
			hls_res.type = HighLevelCreationType::INTEGER;
			hls_res.path = gen_random(5); // TODO make sure this is unique
			int radio_reg_id_ = this->radio()->template reg_resource_callback<self_type, &self_type::dummy >( this->path().append("/").append(hls_res.path), this );
			cout << "Creating new High Level State Resource at path: " << this->path().c_str() << "/" << hls_res.path << "\n";

			for(; it != hl_list.end(); ++it)
			{
				OpaqueData hl_data = (*it);
				uint8_t * option = hl_data.value();
				uint8_t type = option[0];
				if ( type == HighLevelCreationType::INTEGER )
				{
					uint16_t lower = 0 | (option[1]<<8) | option[2];
					uint16_t upper = 0 | (option[3]<<8) | option[4];
					char* name = (char*) option+5;
					char* name_copy = new char[hl_data.length()-5+1];
					memcpy(name_copy, name, hl_data.length()-5);
					cout << "Created integer state:\"" << name << "\" lower: " << lower << " upper: " << upper << "\n";

					number_state<value_t> state = {lower,upper,name_copy};

					hls_res.states.push_back(state);
				}
				else if ( type == HighLevelCreationType::FLOAT )
				{

				}
			}
			state_resources_.push_back(hls_res);

			coap_packet_t repl;
			repl.set_option(COAP_OPT_LOCATION_PATH, hls_res.path);
			this->radio()->reply( msg, 0, 0, COAP_CODE_CREATED, COAP_CONTENT_TYPE_APPLICATION_JSON, repl );
		}
	}

	/**
	 * \brief Generates a random alphanumeric string with a given length.
	 * @param len length of random string
	 * @return Null terminated char array containing the generated string
	 */
	char * gen_random(const size_t len) {
		char* s = new char[len+1];
	    static const char alphanum[] =
	        "0123456789"
	        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	        "abcdefghijklmnopqrstuvwxyz";

	    for (size_t i = 0; i < len; ++i) {
	    	uint32_t ran = (*rand_)( sizeof(alphanum)-1 );
	        s[i] = (char) alphanum[ran];
	    }

	    s[len] = 0;
	    return s;
	}

	/**
	 * \brief Find a State-Resource in the list of all registered State-Resources by a given URI-path
	 * @param uri_path
	 * @return A pointer to the found state resource or NULL if none
	 */
	state_resource_t* find_state_resource(string_t uri_path)
	{
		for ( size_t i=0; i<state_resources_.size(); i++)
		{
			state_resource_t curr = state_resources_.at(i);
			string_t full_path = string_t( this->path() ).append("/").append(curr.path);
			if ( full_path == uri_path )
			{
				return &(state_resources_.at(i));
			}
		}
		return NULL;
	}

	/**
	 * \brief Converts all State-Resources to a description containing paths and mappings in JSON
	 * 			format
	 * @return a char array (max length 1024bytes) containing the JSON
	 */
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

	// TODO externalize status to coap_service
	value_t status()
	{
		return status_;
	}

	void set_status(value_t newStatus)
	{
		status_ = newStatus;
	}

private:
	Timer *timer_;
	Rand *rand_;
	value_t num_;
	value_t status_;

	state_resources_vector_t state_resources_;

};

}

#endif /* STATES_TEST_H_ */
