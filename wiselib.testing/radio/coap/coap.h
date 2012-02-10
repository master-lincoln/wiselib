#ifndef COAP_H
#define COAP_H


//#define DEBUG_COAPRADIO


#include "external_interface/external_interface.h"
#include "util/serialization/simple_types.h"
#include "util/pstl/static_string.h"
#include "util/pstl/list_static.h"
using namespace std;

// Config Tweaks
// out of all the uint options (content type, max age, uri port and accept) only accept can occur multiple times
// and currently (draft-07) there are only six media types. So 8 seemed a pretty good default
#define COAP_LIST_SIZE_UINT		8
// TODO: Sinnvollen Default finden
#define COAP_LIST_SIZE_STRING		16
// TODO: Sinnvollen Default finden
#define COAP_LIST_SIZE_OPAQUE		8
// size of the payload in bytes. Keep in mind, that this is multiplied by the number of coap messages the coap radio keeps in its buffer
#define COAP_DATA_SIZE		250

// Size of message buffer that saves sent and received messages for a while
#define COAPRADIO_SENT_LIST_SIZE		25
#define COAPRADIO_RECEIVED_LIST_SIZE		25
#define COAPRADIO_RESOURCES_SIZE		8
#define COAPRADIO_TIMER_ACTION_SIZE		25


enum CoapMsgIds
{
	CoapMsgId = 51 // Coap Message Type according to Wiselibs Reserved Message IDs
};

// Constants defined in draft-ietf-core-coap-07
#define COAP_VERSION	1
#define COAP_STD_PORT	5683

static const uint16_t COAP_RESPONSE_TIMEOUT = 2000;
static const float COAP_RANDOM_FACTOR = 1.5;
static const uint16_t COAP_MAX_RESPONSE_TIMEOUT = (uint16_t) COAP_RESPONSE_TIMEOUT * COAP_RANDOM_FACTOR;
static const uint8_t COAP_MAX_RETRANSMIT = 4;
// Time before an ACK is sent. This is to give the application a chance to send a piggybacked response
static const uint16_t COAP_ACK_GRACE_PERIOD = COAP_RESPONSE_TIMEOUT / 4;

enum CoapType
{
	COAP_MSG_TYPE_CON = 0,
	COAP_MSG_TYPE_NON = 1,
	COAP_MSG_TYPE_ACK = 2,
	COAP_MSG_TYPE_RST = 3
};

#define COAP_LONG_OPTION	15

enum CoapOptions
{
	COAP_OPT_CONTENT_TYPE = 1,
	COAP_OPT_MAX_AGE = 2,
	COAP_OPT_PROXY_URI = 3,
	COAP_OPT_ETAG = 4,
	COAP_OPT_URI_HOST = 5,
	COAP_OPT_LOCATION_PATH = 6,
	COAP_OPT_URI_PORT = 7,
	COAP_OPT_LOCATION_QUERY = 8,
	COAP_OPT_URI_PATH = 9,
	COAP_OPT_TOKEN = 11,
	COAP_OPT_ACCEPT = 12,
	COAP_OPT_IF_MATCH = 13,
	COAP_OPT_FENCEPOST = 14,
	COAP_OPT_URI_QUERY = 15,
	COAP_OPT_IF_NONE_MATCH = 21
};

#define COAP_OPT_MAXLEN_FENCEPOST		0
#define COAP_OPT_MAXLEN_CONTENT_TYPE	2
#define COAP_OPT_MAXLEN_MAX_AGE			4
#define COAP_OPT_MAXLEN_ETAG			8
#define COAP_OPT_MAXLEN_URI_PORT		2
#define COAP_OPT_MAXLEN_TOKEN			8
#define COAP_OPT_MAXLEN_ACCEPT			2
#define COAP_OPT_MAXLEN_IF_MATCH		8
#define COAP_OPT_MAXLEN_IF_NONE_MATCH	0
#define COAP_STRING_OPTS_MAXLEN			270
#define COAP_STRING_OPTS_MINLEN			1

#define COAP_DEFAULT_MAX_AGE		60


// Finding the longest opaque option, out of the three opage options Etag, Token and IfMatch
#if COAP_OPT_MAXLEN_ETAG > COAP_OPT_MAXLEN_TOKEN
	#if COAP_OPT_MAXLEN_ETAG > COAP_OPT_MAXLEN_IF_MATCH
		#define COAP_OPT_MAXLEN_OPAQUE	COAP_OPT_MAXLEN_ETAG
	#else
		#define COAP_OPT_MAXLEN_OPAQUE	COAP_OPT_MAXLEN_IF_MATCH
	#endif
#else
	#define COAP_OPT_MAXLEN_OPAQUE	COAP_OPT_MAXLEN_TOKEN
#endif

// message codes
//requests
#define COAP_REQUEST_CODE_RANGE_MIN			1
#define COAP_REQUEST_CODE_RANGE_MAX			31

#define COAP_RESPONSE_CODE_RANGE_MIN		64
#define COAP_RESPONSE_CODE_RANGE_MAX		191

enum CoapCode
{
	COAP_CODE_EMPTY = 0,
	COAP_CODE_GET = 1,
	COAP_CODE_POST = 2,
	COAP_CODE_PUT = 3,
	COAP_CODE_DELETE = 4,
	//responses
	COAP_CODE_CREATED = 65, // 2.01
	COAP_CODE_DELETED = 66, // 2.02
	COAP_CODE_VALID = 67, // 2.03
	COAP_CODE_CHANGED = 68, // 2.04
	COAP_CODE_CONTENT = 69, // 2.05
	COAP_CODE_BAD_REQUEST = 128, // 4.00
	COAP_CODE_UNAUTHORIZED = 129, // 4.01
	COAP_CODE_BAD_OPTION = 	130, // 4.02
	COAP_CODE_FORBIDDEN = 131, // 4.03
	COAP_CODE_NOT_FOUND = 132, // 4.04
	COAP_CODE_METHOD_NOT_ALLOWED = 133, // 4.05
	COAP_CODE_NOT_ACCEPTABLE = 134, // 4.06
	COAP_CODE_PRECONDITION_FAILED = 140, // 4.12
	COAP_CODE_REQUEST_ENTITY_TOO_LARGE = 141, // 4.13
	COAP_CODE_UNSUPPORTED_MEDIA_TYPE = 143, // 4.15
	COAP_CODE_INTERNAL_SERVER_ERROR	= 160, // 5.00
	COAP_CODE_NOT_IMPLEMENTED = 161, // 5.01
	COAP_CODE_BAD_GATEWAY = 162, // 5.02
	COAP_CODE_SERVICE_UNAVAILABLE = 163, // 5.03
	COAP_CODE_GATEWAY_TIMEOUT = 164, // 5.04
	COAP_CODE_PROXYING_NOT_SUPPORTED = 165 // 5.05
};

enum TimerType
{
	TIMER_NONE,
	TIMER_RETRANSMIT,
	TIMER_ACK
};

#define COAP_START_OF_OPTIONS	4

#define COAP_FORMAT_NONE		0
#define COAP_FORMAT_UNKNOWN		255
#define COAP_FORMAT_UINT		1
#define COAP_FORMAT_STRING		2
#define COAP_FORMAT_OPAQUE		3
#define COAP_LARGEST_OPTION_NUMBER	21
#define COAP_OPTION_FORMAT_ARRAY_SIZE	COAP_LARGEST_OPTION_NUMBER + 1

static const uint8_t COAP_OPTION_FORMAT[COAP_OPTION_FORMAT_ARRAY_SIZE] =
{
	COAP_FORMAT_UNKNOWN,			// 0: not in use
	COAP_FORMAT_UINT,			// 1: COAP_OPT_CONTENT_TYPE
	COAP_FORMAT_UINT,			// 2: COAP_OPT_MAX_AGE
	COAP_FORMAT_STRING,			// 3: COAP_OPT_PROXY_URI
	COAP_FORMAT_OPAQUE,			// 4: COAP_OPT_ETAG
	COAP_FORMAT_STRING,			// 5: COAP_OPT_URI_HOST
	COAP_FORMAT_STRING,			// 6: COAP_OPT_LOCATION_PATH
	COAP_FORMAT_UINT,			// 7: COAP_OPT_URI_PORT
	COAP_FORMAT_STRING,			// 8: COAP_OPT_LOCATION_QUERY
	COAP_FORMAT_STRING,			// 9: COAP_OPT_URI_PATH
	COAP_FORMAT_UNKNOWN,		// 10: not in use
	COAP_FORMAT_OPAQUE,			// 11: COAP_OPT_TOKEN
	COAP_FORMAT_UINT,			// 12: COAP_OPT_ACCEPT
	COAP_FORMAT_OPAQUE,			// 13: COAP_OPT_IF_MATCH
	COAP_FORMAT_NONE,			// 14: COAP_OPT_FENCEPOST
	COAP_FORMAT_STRING,			// 15: COAP_OPT_URI_QUERY
	COAP_FORMAT_UNKNOWN,		// 16: not in use
	COAP_FORMAT_UNKNOWN,		// 17: not in use
	COAP_FORMAT_UNKNOWN,		// 18: not in use
	COAP_FORMAT_UNKNOWN,		// 19: not in use
	COAP_FORMAT_UNKNOWN,		// 20: not in use
	COAP_FORMAT_NONE			// 21: COAP_OPT_IF_NONE_MATCH
};

static const bool COAP_OPT_CAN_OCCUR_MULTIPLE[COAP_OPTION_FORMAT_ARRAY_SIZE] =
{
	false,			// 0: not in use
	false,			// 1: COAP_OPT_CONTENT_TYPE
	false,			// 2: COAP_OPT_MAX_AGE
	true,			// 3: COAP_OPT_PROXY_URI
	true,			// 4: COAP_OPT_ETAG -- can occur multiple times in Requests, but only once in a response
	false,			// 5: COAP_OPT_URI_HOST
	true,			// 6: COAP_OPT_LOCATION_PATH
	false,			// 7: COAP_OPT_URI_PORT
	true,			// 8: COAP_OPT_LOCATION_QUERY
	true,			// 9: COAP_OPT_URI_PATH
	false,			// 10: not in use
	false,			// 11: COAP_OPT_TOKEN
	true,			// 12: COAP_OPT_ACCEPT
	true,			// 13: COAP_OPT_IF_MATCH
	false,			// 14: COAP_OPT_FENCEPOST
	true,			// 15: COAP_OPT_URI_QUERY
	false,			// 16: not in use
	false,			// 17: not in use
	false,			// 18: not in use
	false,			// 19: not in use
	false,			// 20: not in use
	false			// 21: COAP_OPT_IF_NONE_MATCH
};

namespace wiselib
{
	typedef uint16_t coap_msg_id_t;
	// Size of tokens sent by coapradio.h. This does not affect what size tokens coapradio can receive/process!
	typedef uint32_t coap_token_t;


	class OpaqueData
	{
	public:
		OpaqueData& operator=( const OpaqueData &rhs )
		{
			// avoid self-assignment
			if(this != &rhs)
			{
				set( rhs.value(), rhs.length() );
			}
			return *this;
		}

		bool operator==( const OpaqueData &other ) const
		{
			if( this->length() == other.length() )
			{
				for( size_t i = 0; i < this->length(); ++i )
				{
					if( *( this->value() + i ) != *( other.value() + i ) )
						return false;
				}
				return true;
			}
			return false;
		}

		bool operator!=( const OpaqueData &other ) const
		{
			return !( *this == other );
		}

		OpaqueData()
		{
			length_ = 0;
		}

		OpaqueData( const OpaqueData & rhs)
		{
			*this = rhs;
		}

		OpaqueData( uint8_t * value, size_t length )
		{
			set( value, length );
		}

		~OpaqueData()
		{

		}

		void set( const uint8_t *value, size_t length)
		{
			//TODO: check if length exceeds COAP_OPT_MAXLEN_OPAQUE ?
			memcpy(value_, value, length);
			length_ = length;
		}

		const void get(uint8_t *value, size_t &length) const
		{
			memcpy(value, value_, length_);
			length = length_;
		}

		size_t length() const
		{
			return length_;
		}

		uint8_t * value()
		{
			return value_;
		}

		const uint8_t * value() const
		{
			return value_;
		}

	private:
		uint8_t value_[COAP_OPT_MAXLEN_OPAQUE];
		size_t length_;
	};
}

#endif // COAP_H
