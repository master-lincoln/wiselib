#ifndef COAP_PACKET_H
#define COAP_PACKET_H

#include "coap.h"

#define SINGLE_OPTION_NO_HEADER		0

namespace wiselib
{
	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_ = COAP_DEFAULT_STORAGE_SIZE >
	class CoapPacket
	{
	public:
		typedef OsModel_P OsModel;
		typedef typename OsModel_P::Debug Debug;
		typedef Radio_P Radio;
		typedef typename Radio::block_data_t block_data_t;
		typedef String_T string_t;

		typedef CoapPacket<OsModel_P, Radio_P, String_T, storage_size_> self_type;
		typedef self_type* self_pointer_t;
		typedef self_type coap_packet_t;

		///@name Construction / Destruction
		///@{
		CoapPacket( );
		CoapPacket( const coap_packet_t &rhs );
		~CoapPacket();
		///@}

		coap_packet_t& operator=(const coap_packet_t &rhs);

		void init();

		/**
		 * Takes a stream of data and tries to parse it into the CoapPacket from which this method is called
		 * @param datastream the serial data to be parsed
		 * @param length length of the datastream
		 * @return error code
		 */
		int parse_message( block_data_t *datastream, size_t length );

		/**
		 * Returns the CoAP version number of the packet
		 * @return CoAP version number
		 */
		uint8_t version() const;

		/**
		 * Sets the CoAP version number of the packet. DO NOT set it to anything other than COAP_VERSION unless you know what you are doing!
		 * Note: the corresponding field in the CoAP packet is 2 bit wide. Setting it to anything greater than 3 is pointless.
		 * @param version version number
		 */
		void set_version( uint8_t version );

		/**
		 * Returns the type of the packet. Can be Confirmable, Non-Confirmable, Acknowledgement or Reset.
		 * @return message type
		 */
		CoapType type() const;

		/**
		 * Sets the type of the packet. Can be COAP_MSG_TYPE_CON, COAP_MSG_TYPE_NON, COAP_MSG_TYPE_ACK or COAP_MSG_TYPE_RST.
		 * @param type message type
		 */
		void set_type( CoapType type );

		/**
		 * Returns the code of the packet. Can be a request (1-31), response (64-191) or empty (0). (all other values are reserved)
		 * For details refer to the CoAP Code Registry section of the CoAP draft.
		 * @return code of the message
		 */
		CoapCode code() const;

		/**
		 * Returns whether the packet is a request
		 * @return true if packet is a request, false otherwise
		 */
		bool is_request() const;

		/**
		 * Returns whether the packet is a response
		 * @return true if packet is a response, false otherwise
		 */
		bool is_response() const;

		/**
		 * Sets the code of the packet. Can be a request (1-31), response (64-191) or empty (0). (all other values are reserved)
		 * For details refer to the CoAP Code Registry section of the CoAP draft.
		 * @param code code of the packet
		 */
		void set_code( CoapCode code );

		/**
		 * Returns the message ID by which message duplication can be detected and request/response matching can be done.
		 * @return the message ID
		 */
		coap_msg_id_t msg_id() const;

		/**
		 * Sets the message ID by which message duplication can be detected and request/response matching can be done.
		 * @param msg_id new message ID
		 */
		void set_msg_id( coap_msg_id_t msg_id );

		uint32_t what_options_are_set() const;

		/**
		 * Returns the token by which request/response matching can be done.
		 * @param token reference to OpaqueData object, will contain message token afterwards
		 */
		void token( OpaqueData &token );

		/**
		 * Sets the token by which request/response matching can be done.
		 * @param token new message ID
		 */
		void set_token( const OpaqueData &token );

		string_t uri_path();

		int set_uri_path( string_t &path );

		int set_uri_query( string_t &query );

		template<typename list_t>
		int get_query_list( CoapOptionNum optnum, list_t &result );

		void set_uri_port( uint32_t port );

		uint32_t uri_port();

		/**
		 * Returns a pointer to the payload section of the packet
		 * @return pointer to payload
		 */
		block_data_t* data();
		const block_data_t* data() const;

		/**
		 * Returns the length of the payload
		 * @return payload length
		 */
		size_t data_length() const;

		/**
		 * Sets the payload
		 * @param data the payload
		 * @param length payload length
		 * @return ERR_NOMEM if there is not enough space for data this size
		 * SUCCESS otherwise
		 */
		int set_data( block_data_t* data , size_t length);

		int set_option( CoapOptionNum option_number, uint32_t value );
		int set_option( CoapOptionNum option_number, const string_t &value );
		int set_option( CoapOptionNum option_number, const OpaqueData &value );

		int add_option( CoapOptionNum option_number, uint32_t value );
		int add_option( CoapOptionNum option_number, const string_t &value );
		int add_option( CoapOptionNum option_number, const OpaqueData &value );

		int get_option( CoapOptionNum option_number, uint32_t &value );
		int get_option( CoapOptionNum option_number, string_t &value );
		int get_option( CoapOptionNum option_number, OpaqueData &value );

		int remove_option( CoapOptionNum option_number );

		bool opt_if_none_match() const;
		int set_opt_if_none_match( bool opt_if_none_match );

		size_t option_count() const;

		/**
		 * Calculates the length of the message if it were serialized in the current sate
		 * @return the expected length of the serialized message
		 */
		size_t serialize_length() const;

		/**
		 * Serializes the packet so it can be sent over the radio.
		 * @param pointer to where the serialized packet will be written.
		 * @return length of the packet
		 */
		size_t serialize( block_data_t *datastream ) const;

		void get_error_context( CoapCode &error_code, CoapOptionNum &error_option);

		enum error_code
		{
			// inherited from concepts::BasicReturnValues_concept
			SUCCESS = OsModel::SUCCESS,
			ERR_NOMEM = OsModel::ERR_NOMEM,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
			// coap_packet_t errors
			ERR_WRONG_TYPE,
			ERR_UNKNOWN_OPT,
			ERR_OPT_NOT_SET,
			ERR_OPT_TOO_LONG,
			ERR_METHOD_NOT_APPLICABLE,
			// packet parsing errors
			ERR_OPTIONS_EXCEED_PACKET_LENGTH,
			ERR_UNKNOWN_CRITICAL_OPTION,
			ERR_MULTIPLE_OCCURENCES_OF_CRITICAL_OPTION,
			ERR_EMPTY_STRING_OPTION,
			ERR_NOT_COAP,
			ERR_WRONG_COAP_VERSION
		};


	private:
#ifdef DEBUG_COAPRADIO_TEST_XX
	public:
#endif
		// points to beginning of payload
		block_data_t *payload_;
		// marks the first byte PAST the last option
		block_data_t *end_of_options_;
		size_t data_length_;
		size_t option_count_;
		coap_msg_id_t msg_id_;
		CoapType type_;
		CoapCode code_;
		// only relevant when an error occurs
		CoapCode error_code_;
		CoapOptionNum error_option_;
		// Coap Version
		uint8_t version_;

		block_data_t* options_[COAP_OPTION_ARRAY_SIZE];
		// contains Options and Data
		block_data_t storage_[storage_size_];

		int add_option(CoapOptionNum num, const block_data_t *serial_opt, size_t len, size_t num_of_opts = SINGLE_OPTION_NO_HEADER );
		void scan_opts( block_data_t *start, CoapOptionNum prev );
		int initial_scan_opts( size_t num_of_opts, size_t message_length );
		uint8_t next_fencepost_delta(uint8_t previous_opt_number) const;
		bool is_fencepost( CoapOptionNum optnum) const;
		bool is_critical( CoapOptionNum option_number ) const;
		size_t optlen(block_data_t * optheader) const;
		size_t make_segments_from_string( const char *cstr, char delimiter, CoapOptionNum optnum, block_data_t *segments, size_t &num_segments ) const;
		void make_string_from_segments( char delimiter, CoapOptionNum optnum, string_t &result );

	};


	// Implementation starts here
	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>& CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::operator=(const coap_packet_t &rhs)
	{
		if( &rhs != this )
		{
			memcpy( storage_, rhs.storage_, storage_size_ );
			for( size_t i = 0; i < COAP_OPTION_ARRAY_SIZE; ++i)
			{
				if(rhs.options_[i] != NULL )
				{
					options_[i] = storage_ + ( rhs.options_[i] - rhs.storage_ );
				}
			}
			payload_ = storage_ + ( rhs.payload_ - rhs.storage_ );
			end_of_options_ = storage_ + ( rhs.end_of_options_ - rhs.storage_ );
			data_length_ = rhs.data_length_;
			option_count_ = rhs.option_count_;
			msg_id_ = rhs.msg_id_;
			type_ = rhs.type_;
			code_ = rhs.code_;
			error_code_ = rhs.error_code_;
			error_option_ = rhs.error_option_;
			version_ = rhs.version_;
		}
		return *this;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::CoapPacket()
	{
		init();
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::CoapPacket( const coap_packet_t &rhs)
	{
		init();
		*this = rhs;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::~CoapPacket()
	{
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::init()
	{
		version_ = COAP_VERSION;
		// TODO: sinnvollen Default festlegen und dann ein COAP_MSG_DEFAULT_TYPE Makro anlegen oder so
		type_ = COAP_MSG_TYPE_NON;
		code_ = COAP_CODE_EMPTY;
		msg_id_ = 0;
		for(size_t i = 0; i < COAP_OPTION_ARRAY_SIZE; ++i)
		{
			options_[i] = NULL;
		}

		payload_ = storage_ + storage_size_;
		end_of_options_ = storage_;
		data_length_ = 0;
		option_count_ = 0;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::parse_message( block_data_t *datastream, size_t length )
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "parse_message> length " << length << "\n";
#endif
		// clear everything
		init();

		// can this possibly be a coap packet?
		if( length >= COAP_START_OF_OPTIONS )
		{
			uint8_t coap_first_byte = read<OsModel , block_data_t , uint8_t >( datastream );
			version_ = coap_first_byte >> 6 ;
			if( version_ != COAP_VERSION )
			{
				error_code_ = COAP_CODE_NOT_IMPLEMENTED;
				error_option_ = COAP_OPT_NOOPT;
				return ERR_WRONG_COAP_VERSION;
			}
			type_ = (CoapType) ( ( coap_first_byte & 0x30 ) >> 4 );
			size_t option_count = coap_first_byte & 0x0f;
			code_ = (CoapCode) read<OsModel , block_data_t , uint8_t >( datastream +1 );
			msg_id_ = read<OsModel , block_data_t , coap_msg_id_t >( datastream + 2 );
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "parse_message> version " << version_ << " type " << type_ << " code " << code_ << " msg_id " << hex << msg_id_ << dec << "\n";
#endif


			memcpy( storage_, datastream + COAP_START_OF_OPTIONS, length - COAP_START_OF_OPTIONS );

			int status = initial_scan_opts( option_count, length - COAP_START_OF_OPTIONS );
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "parse_message> status " << status << "\n";
#endif
			return status;
		}
		// can't make any sense of it
		return ERR_NOT_COAP;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint8_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::version() const
	{
		return version_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_version( uint8_t version )
	{
		version_ = version & 0x03;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapType CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::type() const
	{
		return type_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_type( CoapType type )
	{
		type_ = type;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	CoapCode CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::code() const
	{
		return code_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_code( CoapCode code )
	{
		code_ = code;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::is_request() const
	{
		return( code_ >= COAP_REQUEST_CODE_RANGE_MIN && code_ <= COAP_REQUEST_CODE_RANGE_MAX );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::is_response() const
	{
		return( code_ >= COAP_RESPONSE_CODE_RANGE_MIN && code_ <= COAP_RESPONSE_CODE_RANGE_MAX );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint16_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::msg_id() const
	{
		return msg_id_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_msg_id( uint16_t msg_id )
	{
		msg_id_ = msg_id;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_data( block_data_t* data , size_t length)
	{
		// we put data at the very end
		block_data_t *payload = storage_ + ( (storage_size_) - length );
		if( payload < end_of_options_  )
			return ERR_NOMEM;
		data_length_ = length;
		payload_ = payload;
		memmove(payload_, data, data_length_ );
		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	typename Radio_P::block_data_t * CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::data()
	{
		return payload_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	const typename Radio_P::block_data_t * CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::data() const
	{
		return payload_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::data_length() const
	{
		return data_length_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint32_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::what_options_are_set() const
	{
		uint32_t result = 0;
		for( size_t i = 0; i < COAP_OPTION_ARRAY_SIZE; ++i )
		{
			if( options_[i] != NULL )
				result |= 1 << i;
		}
		return result;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::token( OpaqueData &token )
	{
		if( get_option( COAP_OPT_TOKEN, token ) != SUCCESS )
		{
			token = OpaqueData();
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_token( const OpaqueData &token )
	{
		remove_option( COAP_OPT_TOKEN );
		if( token != OpaqueData() )
		{
			add_option( COAP_OPT_TOKEN, token );
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	String_T CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::uri_path()
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "uri_path()\n";
#endif
		string_t path = string_t();
		get_option( COAP_OPT_URI_PATH, path );
		return path;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_uri_path( string_t &path )
	{
		return set_option( COAP_OPT_URI_PATH, path );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_uri_query( string_t &query )
	{
		return set_option( COAP_OPT_URI_QUERY, query );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	template<typename list_t>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>
	::get_query_list( CoapOptionNum optnum, list_t &result )
	{
		if ( optnum != COAP_OPT_LOCATION_QUERY
		     && optnum != COAP_OPT_URI_QUERY )
			return ERR_METHOD_NOT_APPLICABLE;

		if( options_[optnum] == NULL )
			return ERR_OPT_NOT_SET;

		result.clear();
		string_t curr_segment;
		block_data_t swap;
		block_data_t *pos = options_[optnum];
		block_data_t *nextpos;
		size_t value_start;
		size_t curr_segment_len;
		do
		{
			curr_segment_len = optlen( pos );

			if( curr_segment_len < 15 )
				value_start = 1;
			else
				value_start = 2;

			nextpos = pos + curr_segment_len + value_start;
			swap = *(nextpos);
			*nextpos = (block_data_t) '\0';
			curr_segment = (char*) ( pos + value_start );
			result.push_back( curr_segment );
			*nextpos = swap;

			pos = nextpos;
		} while ( pos < end_of_options_ && ( swap & 0xf0 ) == 0 );

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_uri_port( uint32_t port )
	{
		remove_option( COAP_OPT_URI_PORT );
		if( port != COAP_STD_PORT )
		{
			add_option( COAP_OPT_URI_PORT, port );
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint32_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::uri_port()
	{
		uint32_t port = COAP_STD_PORT;
		get_option( COAP_OPT_URI_PORT, port );
		return port;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_option( CoapOptionNum option_number, uint32_t value )
	{
		remove_option(option_number);
		return add_option( option_number, value );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_option( CoapOptionNum option_number, const string_t &value )
	{
		remove_option(option_number);
		return add_option( option_number, value );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_option( CoapOptionNum option_number, const OpaqueData &value )
	{
		remove_option(option_number);
		return add_option( option_number, value );
	}

	template<typename OsModel_P,
		typename Radio_P,
		typename String_T,
		size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::add_option( CoapOptionNum option_number, uint32_t value )
	{
		if( option_number > COAP_LARGEST_OPTION_NUMBER )
		{
			return ERR_UNKNOWN_OPT;
		}
		if( COAP_OPTION_FORMAT[option_number] != COAP_FORMAT_UINT )
		{
			return ERR_WRONG_TYPE;
		}
		block_data_t serial[4];
		size_t highest_non_zero_byte = 5;
		serial[ 3 ] = value & 0x000000ff;
		serial[ 2 ] = (value & 0x0000ff00) >> 8;
		serial[ 1 ] = (value & 0x00ff0000) >> 16;
		serial[ 0 ] = (value & 0xff000000) >> 24;
		if( serial[0] != 0 )
			highest_non_zero_byte = 1;
		else if ( serial[1] != 0 )
			highest_non_zero_byte = 2;
		else if ( serial[2] != 0 )
			highest_non_zero_byte = 3;
		else if ( serial[3] != 0 )
			highest_non_zero_byte = 4;

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "add_option(uint)> value: "<< hex << value
		     << ", serial: " << serial[ 0 ] << ", " << serial[ 1 ] << ", "
		     << serial[ 2 ] << ", " << serial[ 3 ]
		     << ", highest non-zero byte: " << dec << highest_non_zero_byte
		     << "\n";
#endif

		return add_option( option_number,
		                   serial + highest_non_zero_byte - 1,
		                   5 - highest_non_zero_byte );
	}

	template<typename OsModel_P,
		typename Radio_P,
		typename String_T,
		size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::add_option( CoapOptionNum option_number, const string_t &value )
	{
		if( option_number > COAP_LARGEST_OPTION_NUMBER )
		{
			return ERR_UNKNOWN_OPT;
		}
		if( COAP_OPTION_FORMAT[option_number] != COAP_FORMAT_STRING )
		{
			return ERR_WRONG_TYPE;
		}
		if( value.length() == 0 )
		{
			return SUCCESS;
		}

		// reserve some additional space for option headers
		block_data_t insert[ value.length() + 20 ];
		size_t num_segments = 0;
		size_t serial_len = 0;
		size_t segment_start = 0;

		const char* c_str = (const_cast<string_t&>(value)).c_str();

		if( option_number == COAP_OPT_LOCATION_PATH
		   || option_number  == COAP_OPT_URI_PATH )
		{
			if( (size_t) value.length() > segment_start && c_str[segment_start] == '/' )
				++segment_start;
			if( (size_t) value.length() > segment_start )
			{
				serial_len = make_segments_from_string(c_str + segment_start,
				          '/', option_number, insert, num_segments );
			}
			else
				return SUCCESS;
		}
		else if ( option_number == COAP_OPT_LOCATION_QUERY
		         || option_number  == COAP_OPT_URI_QUERY )
		{
			if( (size_t) value.length() > segment_start && c_str[segment_start] == '/' )
				++segment_start;
			if( (size_t) value.length() > segment_start && c_str[segment_start] == '?' )
				++segment_start;
			if( (size_t) value.length() > segment_start )
			{
				serial_len = make_segments_from_string( c_str + segment_start,
				             '&', option_number, insert, num_segments );
			}
			else
				return SUCCESS;
		}
		else
		{
			if( value.length() <= COAP_STRING_OPTS_MAXLEN )
			{
				memcpy(insert, c_str, value.length());
				serial_len = value.length();
				num_segments = 0;
			}
			else
			{
				size_t copylen = 0;
				do
				{
					*(insert + serial_len) = COAP_LONG_OPTION;
					++serial_len;
					*(insert + serial_len) = 0xff;
					++serial_len;
					copylen = value.length() - num_segments * 270;
					if( copylen > 270 )
						copylen = 270;
					memcpy( insert + serial_len,
					        c_str + segment_start, copylen);
					serial_len += copylen;
					++num_segments;
					segment_start += copylen;
				} while( segment_start < (size_t) value.length() );
			}
		}

		return add_option( option_number, insert, serial_len, num_segments );
	}

	template<typename OsModel_P,
		typename Radio_P,
		typename String_T,
		size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::add_option( CoapOptionNum option_number, const OpaqueData &value)
	{
		if( option_number > COAP_LARGEST_OPTION_NUMBER )
		{
			return ERR_UNKNOWN_OPT;
		}
		if( COAP_OPTION_FORMAT[option_number] != COAP_FORMAT_OPAQUE )
		{
			return ERR_WRONG_TYPE;
		}

		return add_option( option_number, value.value(), value.length() );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::get_option( CoapOptionNum option_number, uint32_t &value )
	{
		if( options_[option_number] != NULL )
		{
			value = 0;
			block_data_t *raw = options_[option_number];
			size_t len = *raw & 0xf;
			size_t pos = 1;

			switch( len )
			{
			case 4:
				value = *(raw + pos);
				++pos;
			case 3:
				value = (value << 8) | (raw + pos);
				++pos;
			case 2:
				value = (value << 8) | (raw + pos);
				++pos;
			case 1:
				value = (value << 8) | (raw + pos);
				++pos;
			case 0:
				// do nothing
				break;
			default:
				// uint option longer than 4. SHOULD NOT HAPPEN!!!
				return ERR_OPT_TOO_LONG;
			}
			return SUCCESS;
		}
		return ERR_OPT_NOT_SET;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::get_option( CoapOptionNum option_number, string_t &value )
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "get_option( " << (int) option_number << ", string_t ), options_[ "
		     << option_number << " ] = " << hex << (int) options_[option_number]
		     << dec << "\n";
#endif
		if( options_[option_number] == NULL )
			return ERR_OPT_NOT_SET;

		if( option_number == COAP_OPT_LOCATION_PATH
				|| option_number  == COAP_OPT_URI_PATH )
		{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "get_option( " << (int) option_number << ", string_t )> *_PATH\n";
#endif
			make_string_from_segments ( '/', option_number, value );
		}
		else if ( option_number == COAP_OPT_LOCATION_QUERY
				|| option_number  == COAP_OPT_URI_QUERY )
		{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "get_option( " << (int) option_number << ", string_t )> *_QUERY\n";
#endif
			make_string_from_segments ( '&', option_number, value );
		}
		else
		{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "get_option( " << (int) option_number << ", string_t )> else\n";
#endif
			// TODO: testen ob das funtioniert
			make_string_from_segments ( '\0', option_number, value );
		}
		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::get_option( CoapOptionNum option_number, OpaqueData &value )
	{
		if( options_[option_number] == NULL )
			return ERR_OPT_NOT_SET;

		size_t len = optlen( options_[option_number] );
		size_t value_start = (len >= 15) ? 2 : 1;

		value.set( options_[option_number] + value_start, len );

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::remove_option( CoapOptionNum option_number )
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "remove_option( " << (int) option_number << " )> removing opt \n";
#endif
		block_data_t *removal_start = options_[option_number];
		size_t removal_len = 0;
		size_t num_segments = 0;
		size_t curr_segment_len;
		if( removal_start != NULL )
		{
			do
			{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
				cout << "remove_option( " << (int) option_number << " )> Num of Segments: " << num_segments << "\n";
#endif
				++num_segments;
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
				cout << "remove_option( " << (int) option_number << " )> Num of Segments: " << num_segments << "\n";
#endif
				curr_segment_len = *(removal_start + removal_len) & 0x0f;
				if( curr_segment_len == COAP_LONG_OPTION )
				{
					curr_segment_len += *(removal_start + removal_len + 1) + 2;
				}
				else
				{
					++curr_segment_len;
				}

				removal_len += curr_segment_len;
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
				cout << "remove_option( " << (int) option_number << " )> Do-while pass " << num_segments << "\n";
#endif
			} while( (removal_start + removal_len) < end_of_options_
			         && ( *(removal_start + removal_len) & 0xf0 ) == 0 );

			// insert a new fencepost if necessary
			if( (removal_start + removal_len) < end_of_options_ )
			{
				if( ( ( (*removal_start) & 0xf0 ) >> 4)
				    + ( ( *(removal_start + removal_len)  & 0xf0 ) >> 4 ) )
				{
					*removal_start = next_fencepost_delta( option_number
					                 - ( ( (*removal_start) & 0xf0 ) >> 4) ) << 4;
					++removal_start;
					--removal_len;
					--num_segments;
				}
			}

			// if the removed option is the last option and the one
			// before is a fencepost, remove the fencepost
			if( (removal_start + removal_len) >= end_of_options_ )
			{
				CoapOptionNum prev = (CoapOptionNum) (option_number - ( ((*removal_start) & 0xf0 ) >> 4));
				if(is_fencepost( prev ))
				{
					removal_start = options_[ prev ];
					removal_len = (size_t) (end_of_options_ - removal_start);
					options_[ prev ] = NULL;
					++num_segments;
				}
			}

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "entferne " << removal_len << " bytes in "
		     << num_segments << " segmenten" << "\n";
		cout << "memmove bewegt "
		     << size_t (end_of_options_ - (removal_start + removal_len) )
		     << " bytes\n";
#endif

			memmove( removal_start,
					removal_start + removal_len,
					size_t (end_of_options_ - (removal_start + removal_len) ) );
			options_[ option_number ] = NULL;
			option_count_ -= num_segments;
			end_of_options_ -= removal_len;
		}
		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::opt_if_none_match() const
	{
		return options_[COAP_OPT_IF_NONE_MATCH] != NULL;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::set_opt_if_none_match( bool opt_if_none_match )
	{
		if( opt_if_none_match )
		{
			return add_option(COAP_OPT_IF_NONE_MATCH , NULL, 0 );
		}
		else
		{
			return remove_option( COAP_OPT_IF_NONE_MATCH );
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>
	::option_count() const
	{
//		size_t count = 0;
//		block_data_t * position = (block_data_t*) storage_;
//		size_t len = 0;
//		while( position < end_of_options_ )
//		{
//			len = (*position) & 0x0f;
//			if( len == 15 )
//			{
//				len = 16 + ( *(position + 1) );
//			}
//			position += len + 1;
//			++count;
//		}
//		return count;
		return option_count_;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::serialize_length() const
	{
		// header (4 bytes) + options + payload
		return (size_t) ( 4 +
		         ( end_of_options_ - storage_ ) +
		         ( storage_ + ( storage_size_ ) - payload_ ));
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>
	::serialize( block_data_t *datastream ) const
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "serialize> version " << version_ << " type " << type_ << " code " << code_ << " msg_id " << hex << msg_id_ << dec << "\n";
#endif
		datastream[0] = ((version() & 0x03) << 6) | ((type() & 0x03) << 4) | (( option_count()) & 0x0f);
		datastream[1] = code();
		datastream[2] = (this->msg_id() & 0xff00) >> 8;
		datastream[3] = (this->msg_id() & 0x00ff);

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "serialize> " << hex << datastream[0] << " " << datastream[1] << " " << datastream[2] << " " << datastream[3] << " " << dec << "\n";
#endif

		size_t len = 4;
		memcpy( datastream + len, storage_, (size_t) (end_of_options_ - storage_) );
		len += (size_t) (end_of_options_ - storage_);
		memcpy( datastream + len,
		        payload_, data_length_ );
		len += (size_t) ((storage_ + storage_size_) - payload_ );

		return len;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::get_error_context( CoapCode &error_code, CoapOptionNum &error_option)
	{
		error_code = error_code_;
		error_option = error_option_;
	}

//-----------------------------------------------------------------------------
// Private methods start here

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::add_option(CoapOptionNum num, const block_data_t *serial_opt, size_t len, size_t num_of_opts)
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "add_option( " << (int) num << " )> len " << len << " num_of_opts " << num_of_opts << "\n";
#endif
		// TODO: evtl vorangehende und folgende fenceposts entfernen
		CoapOptionNum prev = (CoapOptionNum) 0;
		CoapOptionNum next = (CoapOptionNum) 0;
		uint8_t fencepost = 0;
		// only add header when a single option is added
		size_t overhead_len = 0;
		if( num_of_opts == SINGLE_OPTION_NO_HEADER )
		{
			overhead_len = 1;
			if( len >= COAP_LONG_OPTION )
				++overhead_len;
		}

		size_t fenceposts_omitted_len = 0;

		block_data_t *put_here = end_of_options_;
		block_data_t *next_opt_start = put_here;
		// there are options set
		if( put_here > storage_ )
		{
			// look for the next bigger option - this is where we need to start
			// moving things further back
			for( size_t i = (size_t) num + 1; i < COAP_OPTION_ARRAY_SIZE; ++i )
			{
				if( options_[i] != NULL )
				{
					next = (CoapOptionNum) i;
					put_here = options_[i];

					if( is_fencepost( next ) )
					{
						// if the delta to the option after the fencepost is
						// small enough, we can ommit the fencepost
						CoapOptionNum nextnext = (CoapOptionNum) ( next +
						        // get the next option header
								// considering the fenceposts length is only a
								// precaution, it should be zero
						        ( ( *( options_[next]
						        + ( *(options_[next]) & 0x0f ) + 1 )
						        // bitwise AND and shift to get the delta
						        & 0xf0) >> 4 ));

						if( nextnext - num <= 15 )
						{
							options_[next] = NULL;
							next = nextnext;
						}
					}
					next_opt_start = options_[next];

					break;
				}
			}

			// look for previous option - can be the same option we're inserting
			if( next != 0 )
			{
				prev = (CoapOptionNum) ( next - (CoapOptionNum) ( ( *(options_[next]) & 0xf0 ) >> 4 ));
			}
			else
			{
				for( size_t i = (size_t) num; i > 0; --i )
				{
					if( options_[i] != NULL )
					{
						prev = (CoapOptionNum) i;
						break;
					}
				}
			}

			if( is_fencepost( prev ) )
			{
				// if the delta to the option before the fencepost is
				// small enough, we can ommit the fencepost
				CoapOptionNum prevprev = (CoapOptionNum) ( prev -
						( ( *( options_[prev] ) && 0xf0) >> 4 ) );
				if( num - prevprev <= 15 )
				{
					put_here = options_[prev];
					options_[prev] = NULL;
					prev = prevprev;
				}
			}

			fenceposts_omitted_len = next_opt_start - put_here;
		}

		if( num - prev > 15 )
		{
			++overhead_len;
			fencepost = next_fencepost_delta( prev );
		}

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "overhead len: " << overhead_len << ", fp omit len: " << fenceposts_omitted_len << " \n";
		cout << "prev: " << prev << ", next: " << next << "\n";
		cout << "fencepost: " << fencepost << "\n";
#endif

		// move following options back
		size_t bytes_needed = len + overhead_len - fenceposts_omitted_len;
		if( end_of_options_ + bytes_needed >= payload_ )
			return ERR_NOMEM;
		if( put_here < end_of_options_ )
		{
			// correcting delta of following option
			*next_opt_start = ( *next_opt_start & 0x0f )
			                  | (block_data_t) ((next - num) << 4);

			memmove( next_opt_start + bytes_needed,
			        next_opt_start,
			        (size_t) (end_of_options_ - next_opt_start));
		}

		memcpy( put_here + (bytes_needed - len), serial_opt, len );
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "end_of_options_: " << (unsigned int) end_of_options_ <<"\n";
#endif
		end_of_options_ += bytes_needed;
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "end_of_options_: " << (unsigned int) end_of_options_ <<"\n";
#endif
		if( fencepost != 0)
		{
			*put_here = fencepost << 4;
			prev = (CoapOptionNum) fencepost;
			options_[fencepost] = put_here;
			++put_here;
			++option_count_;
		}
		// if multiple options are inserted only add delta, otherwise add size too
		*put_here = ( *put_here & 0x0f ) | ((num - prev) << 4);
		if( num_of_opts == SINGLE_OPTION_NO_HEADER )
		{
			if( len < COAP_LONG_OPTION )
			{
				*put_here = ( *put_here & 0xf0 ) | (len & 0x0f);
			}
			else
			{
				*put_here = ( *put_here & 0xf0 ) | 0x0f;
				*(put_here + 1) = (len - COAP_LONG_OPTION);
			}
		}

		// if we just appended we don't want to overwrite the option's pointer
		if( prev == num )
		{
			put_here += len;
		}
		scan_opts( put_here, prev );

		if( num_of_opts == SINGLE_OPTION_NO_HEADER )
			++option_count_;
		else
			option_count_ += num_of_opts;

		return SUCCESS;
	}



	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::scan_opts( block_data_t *start, CoapOptionNum prev)
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "scan_opts> start: " << (unsigned int) start << ", num: " << prev <<"\n";
#endif

		uint8_t delta = 0;
		while( start < end_of_options_ )
		{
			if( (delta = (((*start) & 0xf0) >> 4)) != 0 )
				options_[ prev + delta ] = start;
			size_t len = optlen( start );
			start += len + 1;
		}
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	int CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::initial_scan_opts( size_t num_of_opts, size_t message_length )
	{
		block_data_t *curr_position = storage_;
		option_count_ = 0;
		CoapOptionNum current = (CoapOptionNum) 0;
		CoapOptionNum previous = (CoapOptionNum) 0;
		size_t opt_length;
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "initial_scan_opts> num opts " << num_of_opts << " length " << message_length <<"\n";
#endif
		while( ( option_count_ < num_of_opts  || num_of_opts == COAP_UNLIMITED_OPTIONS ) )
		{
			// end of options
			if( num_of_opts == COAP_UNLIMITED_OPTIONS
			    && *curr_position == COAP_END_OF_OPTIONS_MARKER )
				break;

			current = (CoapOptionNum) ( previous + ( ( *curr_position & 0xf0) >> 4) );

			// length of option plus header
			opt_length = *curr_position & 0x0f;
			if( opt_length == COAP_LONG_OPTION )
				opt_length = *(curr_position + 1) + 17;
			else
				++opt_length;

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "initial_scan_opts> found option " << (int) current << " length " << opt_length <<"\n";
#endif

			if( current == previous
			    && !COAP_OPT_CAN_OCCUR_MULTIPLE[current] )
			{
				// option is critical
				if ( is_critical( current ) )
				{
					error_code_ = COAP_CODE_BAD_OPTION;
					error_option_ = current;
					return ERR_MULTIPLE_OCCURENCES_OF_CRITICAL_OPTION;
				}
				// otherwise ignore
				++option_count_;
				previous = current;
				curr_position += opt_length;
				continue;
			}

			// check for unknown options
			if ( ( current > COAP_LARGEST_OPTION_NUMBER )
			     || ( COAP_OPTION_FORMAT[current] == COAP_FORMAT_UNKNOWN ) )
			{
				// option is critical
				if ( is_critical( current ) )
				{
					error_code_ = COAP_CODE_BAD_OPTION;
					error_option_ = current;
					return ERR_UNKNOWN_CRITICAL_OPTION;
				}
				// otherwise ignore
				++option_count_;
				previous = current;
				curr_position += opt_length;
				continue;
			}

			if( COAP_OPTION_FORMAT[current] == COAP_FORMAT_STRING
			    && opt_length == 0 )
			{
				if ( is_critical( current ) )
				{
					error_code_ = COAP_CODE_BAD_OPTION;
					error_option_ = current;
					return ERR_EMPTY_STRING_OPTION;
				}
				// otherwise ignore
				++option_count_;
				previous = current;
				curr_position += opt_length;
				continue;
			}

			if( current != previous )
			{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "initial_scan_opts> setting options_[ " << (int) current << " ] = "
		     << hex << (int) curr_position << dec << "\n";
#endif
				options_[ current ] = curr_position;
			}

			++option_count_;
			previous = current;
			curr_position += opt_length;

			if( (curr_position >= (storage_ + storage_size_)
			    || curr_position >= ( storage_ + message_length ))
			    && ( option_count_ < num_of_opts ) )
			{
				error_code_ = COAP_CODE_BAD_REQUEST;
				error_option_ = current;
				return ERR_OPTIONS_EXCEED_PACKET_LENGTH;
			}
		}

		end_of_options_ = curr_position;

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "initial_scan_opts> " << option_count_ << " options found\n";
#endif

		// Rest is data
		if( curr_position < ( storage_ + message_length ) )
		{
			int status = set_data(curr_position,
					message_length - ( curr_position - storage_ ) );
			if( status != SUCCESS )
				return status;
		}

		return SUCCESS;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	uint8_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::next_fencepost_delta(uint8_t previous_opt_number) const
	{
		return ( COAP_OPT_FENCEPOST - ( (previous_opt_number) % COAP_OPT_FENCEPOST ) );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::is_fencepost(CoapOptionNum optnum) const
	{
		return (optnum > 0 && optnum % 14 == 0);
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	bool CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::is_critical( CoapOptionNum option_number ) const
	{
		// odd option numbers are critical
		return( option_number & 0x01 );
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>::optlen(block_data_t * optheader) const
	{
		size_t len = *optheader & 0x0f;
		if( len == COAP_LONG_OPTION )
		{
			len += *(optheader + 1);
		}
		return len;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	size_t CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>
	::make_segments_from_string( const char *cstr, char delimiter, CoapOptionNum optnum, block_data_t *result, size_t &num_segments ) const
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "CoapPacket::add_string_segments> string '"
		     << cstr << "', delimiter " << delimiter << "\n";
#endif
		size_t result_pos = 0;
		size_t position = 0;
		size_t segment_start = 0;
		size_t length = 0;
		num_segments = 0;
		if(cstr[position] == '\0')
			return 0;
		for( ; position <= strlen(cstr); ++position )
		{
			if( cstr[position] == delimiter ||
			    ( cstr[position] == '\0' && cstr[position - 1] != delimiter ) )
			{
				if( (length = position - segment_start ) == 0)
					return 0;

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "CoapPacket::add_string_segments> found segment at "
		     << segment_start << " length " << length << "\n";
#endif
				if( length >= COAP_LONG_OPTION && length <= COAP_STRING_OPTS_MAXLEN )
				{
					*(result + result_pos) = 0x0f;
					++result_pos;
					*(result + result_pos) = (block_data_t) length - COAP_LONG_OPTION;
					++result_pos;
				}
				else if ( length < COAP_LONG_OPTION )
				{
					*(result + result_pos) = (block_data_t) length;
					++result_pos;
				}
				else
					return 0;

				memcpy(result + result_pos, cstr + segment_start, length );
				result_pos += length;
				++num_segments;
				segment_start = position + 1;
			}
		}

#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "CoapPacket::make_segments> found " << num_segments << " segments, wrote "
		     << result_pos << " bytes\n";
#endif
		return result_pos;
	}

	template<typename OsModel_P,
	typename Radio_P,
	typename String_T,
	size_t storage_size_>
	void CoapPacket<OsModel_P, Radio_P, String_T, storage_size_>
	::make_string_from_segments( char delimiter, CoapOptionNum optnum, string_t &result )
	{
#if (defined BOOST_TEST_DECL && defined VERBOSE_DEBUG )
		cout << "CoapPacket::make_string> delimiter " << delimiter << "\n";
#endif
		result = "";
		if( options_[optnum] != NULL )
		{
			char terminated_delimiter[] = { delimiter, '\0' };
			block_data_t swap;
			block_data_t *pos = options_[optnum];
			block_data_t *nextpos;
			size_t value_start;
			size_t curr_segment_len;
			do
			{
				curr_segment_len = optlen( pos );

				if( curr_segment_len < 15 )
					value_start = 1;
				else
					value_start = 2;

				nextpos = pos + curr_segment_len + value_start;
				swap = *(nextpos);
				*nextpos = (block_data_t) '\0';
				result.append( (char*) ( pos + value_start ) );
				*nextpos = swap;

				if( (nextpos < end_of_options_ ) && ( swap & 0xf0 ) == 0 )
				{
					result.append( terminated_delimiter );
				}
				pos = nextpos;
			} while ( pos < end_of_options_ && ( swap & 0xf0 ) == 0 );
		}

	}

}




#endif // COAP_PACKET_H
