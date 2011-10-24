#include "CoapPacket.h"

CoapPacket::CoapPacket()
{
	init();
}

CoapPacket::CoapPacket(uint8_t* datastream, size_t length)
{
	init();
	if(length > 3)
	{
		version_ = datastream[0] >> 6;
		type_ = ( datastream[0] & 0x30 ) >> 4;
		size_t option_count = datastream[0] & 0x0f;
		code_ = datastream[1];
		msg_id_ = ( datastream[2] << 8 ) | datastream[3];

		uint8_t parsed_options = 0;
		uint8_t last_option_number = 0;

		unsigned int i = COAP_START_OF_OPTIONS;
		uint8_t option_number = 0;
		size_t length_of_option = 0;
		// TODO: Überlegen ob das mit der while-Schleife so klug ist
		// bzw. aufpassen, dass man mit den ganzen i++ nicht aus den Options
		// rausläuft
		while( parsed_options < option_count && i < length )
		{
			// TTTTT  OOO   DD    OOO
			//   T   O   O  D D  O   O
			//   T   O   O  D  D O   O
			//   T   O   O  D D  O   O
			//   T    OOO   DD    OOO
			// Diese ganze While-Schleife ist scheisse und fehlerträchtig
			option_number = last_option_number + ( datastream[i] >> 4 );
			last_option_number = option_number;
			length_of_option = datastream[i] & 0x0f;
			if( length_of_option == COAP_LONG_OPTION && i + 1 < length )
			{
				++i;
				length_of_option = datastream[i] + 15;
			}
			++i;
			// TODO: nochmal nachdenken ob das stimmt
			if( i + length_of_option - 1 < length )
			{
				parse_option(option_number, length_of_option, datastream+i);
				++parsed_options;
				i += length_of_option;
			}
			else
			{
				// TODO: Fehlerhaftes Paket behandeln!
			}
		}

		// if there is no data we just leave it at (NULL, 0)
		if(i < length)
		{
			set_data( datastream + i, length - i );
		}

	}
}

CoapPacket::~CoapPacket()
{
	//dtor
}

size_t CoapPacket::serialize(uint8_t *datastream)
{
	return serialize(datastream, this->msg_id());
}

size_t CoapPacket::serialize(uint8_t *datastream, uint16_t msg_id)
{
	datastream[0] = ((version() & 0x03) << 6) | ((type() & 0x03) << 4) | (option_count() & 0x0f);
	datastream[1] = code();
	datastream[2] = (this->msg_id() & 0xff00) >> 8;
	datastream[3] = (this->msg_id() & 0x00ff);

	int offset = 4;

	list<UintOption>::iterator uit = uint_options_.begin();
	list<StringOption>::iterator sit = string_options_.begin();
	list<OpaqueOption>::iterator oit = opaque_options_.begin();

	size_t sindex;
	size_t uindex;
	size_t oindex;
	uint8_t last_option_number = 0;

	while( true )
	{
		if( uit != uint_options_.end() )
		{
			uindex = ( *uit ).option_number();
		}
		else
		{
			uindex = (size_t) - 1;
		}

		if( sit != string_options_.end() )
		{
			sindex = ( *sit ).option_number();
		}
		else
		{
			sindex = (size_t) - 1;
		}

		if( oit != opaque_options_.end() )
		{
			oindex = ( *oit ).option_number();
		}
		else
		{
			oindex = (size_t) - 1;
		}

		// all options have been serialized
		if( uindex == ( (size_t) - 1 )
				&& sindex == ( (size_t) - 1 )
				&& oindex == ( (size_t) - 1 ))
		{
			break;
		}

		if( uindex < sindex )
		{
			if( oindex < uindex )
			{
				offset += serialize_option( datastream + offset, last_option_number, ( *oit ) );
				last_option_number = ( *oit ).option_number();
				++oit;
				continue;
			}
			else
			{
				offset += serialize_option( datastream + offset, last_option_number, ( *uit ) );
				last_option_number = ( *uit ).option_number();
				++uit;
				continue;
			}
		}
		else
		{
			if( oindex < sindex )
			{
				offset += serialize_option( datastream + offset, last_option_number, ( *oit ) );
				last_option_number = ( *oit ).option_number();
				++oit;
				continue;
			}
			else
			{
				offset += serialize_option( datastream + offset, last_option_number, ( *sit ) );
				last_option_number = ( *sit ).option_number();
				++sit;
				continue;
			}
		}
	}

	if(opt_if_none_match())
	{
		FENCEPOSTING( COAP_OPT_IF_NONE_MATCH )
		datastream[offset] = (uint8_t) (( COAP_OPT_IF_NONE_MATCH - last_option_number ) << 4 );
		++offset;
	}

	memcpy( datastream + offset, data(), data_length());
	offset += data_length();
	return offset;
}

size_t CoapPacket::serialize_length()
{
	//TODO: Wenn man Fenceposts braucht um eine Option zu "erreichen" erhöht sich die Länge

	// the header is 4 bytes long
	size_t length = 4;

	list<StringOption>::iterator sit = string_options_.begin();
	for(; sit != string_options_.end(); ++sit)
	{
		// Option header
		++length;
		if( ( *sit ).value().length() > 14 )
		{
			++length;
		}
		length += ( *sit ).value().length();
	}

	list<UintOption>::iterator uit = uint_options_.begin();
	for(; uit != uint_options_.end(); ++uit)
	{
		// Option header
		++length;

		// TODO: Magic Number.... die Uint-Optionen sind in der aktuellen Version (draft-07)
		// des Standards maximal 4 byte lang (Max-Age)
		// TODO: und es gibt sicher einen eleganteren Weg herauszufinden wie viele Bytes
		// man für eine Zahl braucht
		for( int i = 0; i < 4; ++i )
		{
			if ( ( *uit ).value() % (0x100 << (i * 8)) == ( *uit ).value() )
			{
				length += i+1;
				break;
			}
		}
	}

	list<OpaqueOption>::iterator oit = opaque_options_.begin();
	for(; oit != opaque_options_.end(); ++oit)
	{
		// Option header
		++length;
		// TODO: nach aktuellem Standard (draft07) gibt es keine Opaque Options
		// länger als 8 byte, klären ob das so bleibt, oder ob man wie bei
		// String optionen damit rechnen muss, dass die Optionen länger als 14
		// byte werden können
		length += ( *oit ).length();
	}

	length += data_length();

	return length;
}

void CoapPacket::init()
{
	// TODO: _Alle_ Variablen initialisieren und ordentlich machen. Setter benutzen etc.
	this->version_ = COAP_VERSION;
	this->type_ = 0;
	this->code_ = 0;
	this->msg_id_ = 0;

//	this->set_opt_content_type(0);
//	this->set_opt_max_age(COAP_DEFAULT_MAX_AGE);
	this->set_opt_if_none_match(false);

	this->data_length_ = 0;
}

void CoapPacket::parse_option( uint8_t option_number, uint16_t option_length, uint8_t* value)
{
	switch ( option_number )
	{
		case COAP_OPT_CONTENT_TYPE:
			if( option_length <= COAP_OPT_MAXLEN_CONTENT_TYPE )
			{
				set_opt_content_type( 0 );
				uint16_t contype = 0;
				for (int i = 0; i < option_length; ++i)
				{
					opt_content_type(contype);
					set_opt_content_type( (contype << 8) | value[i] );
				}
			}
			else
			{
				// TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_MAX_AGE:
			if( option_length <= COAP_OPT_MAXLEN_MAX_AGE )
			{
				if( option_length > 1 )
				{
					uint32_t maxage = 0;
					for (int i = 0; i < option_length; ++i)
					{
						maxage = ( maxage << 8 ) | value[i];
					}
					set_opt_max_age( maxage);
				}
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_PROXY_URI:
			if( option_length <= COAP_OPT_MAXLEN_PROXY_URI )
			{
				add_opt_proxy_uri( string( (char*) value, option_length ) );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_ETAG:
			if( option_length <= COAP_OPT_MAXLEN_ETAG )
			{
				set_opt_etag( value, option_length );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_URI_HOST:
			if( option_length <= COAP_OPT_MAXLEN_URI_HOST )
			{
				set_opt_uri_host( string( (char*) value, option_length ) );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_LOCATION_PATH:
			if( option_length <= COAP_OPT_MAXLEN_LOCATION_PATH )
			{
				add_opt_location_path( string( (char*) value, option_length ) );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_URI_PORT:
			if( option_length <= COAP_OPT_MAXLEN_URI_PORT )
			{
				uint16_t port = 0;
				for(int i = 0; i < option_length; ++i)
				{
					port = port << 8;
					port |= value[i];
				}
				set_opt_uri_port( port );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_LOCATION_QUERY:
			if( option_length <= COAP_OPT_MAXLEN_LOCATION_QUERY )
			{
				add_opt_location_query( string( (char*) value, option_length ) );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_URI_PATH:
			if( option_length <= COAP_OPT_MAXLEN_URI_PATH )
			{
				add_opt_uri_path( string( (char*) value, option_length ) );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_TOKEN:
			if( option_length <= COAP_OPT_MAXLEN_TOKEN )
			{
				set_opt_token(value, option_length);
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_ACCEPT:
			if( option_length <= COAP_OPT_MAXLEN_ACCEPT )
			{
				uint16_t accept = 0;
				for(int i = 0; i < option_length; ++i)
				{
					accept = accept << 8;
					accept |= value[i];
				}
				add_opt_accept( accept );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_IF_MATCH:
			if( option_length <= COAP_OPT_MAXLEN_IF_MATCH )
			{
				add_opt_if_match( value, option_length );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_URI_QUERY:
			if( option_length <= COAP_OPT_MAXLEN_URI_QUERY )
			{
				add_opt_uri_query( string( (char*) value, option_length ) );
			}
			else
			{
				//TODO: Fehlerbehandlung
			}
			break;
		case COAP_OPT_IF_NONE_MATCH:
				set_opt_if_none_match( true );
			break;
		case COAP_OPT_FENCEPOST:
			// do nothing
			break;
		default:
			// TODO: Fehlerbehandlung unbekannter Option Numbers
			break;
	}
}

// methods for manipulating options
bool CoapPacket::opt_content_type(uint16_t &content_type)
{
	uint32_t value = 0;
	if(get_option(COAP_OPT_CONTENT_TYPE, value))
	{
		content_type = value & 0xffff;
		return true;
	}
	return false;
}

void CoapPacket::set_opt_content_type( uint16_t content_type )
{
	UintOption contype(COAP_OPT_CONTENT_TYPE, content_type);
	set_option(uint_options_, contype);
}

bool CoapPacket::opt_max_age(uint32_t &max_age)
{
	uint32_t value = 0;
	if(get_option(COAP_OPT_CONTENT_TYPE, value))
	{
		max_age = value;
		return true;
	}
	return false;
}

void CoapPacket::set_opt_max_age( uint32_t max_age )
{
	UintOption maxage(COAP_OPT_MAX_AGE, max_age);
	set_option(uint_options_, maxage);
}

bool CoapPacket::opt_proxy_uri( list<string> &proxy_uri )
{
	return get_option_all(COAP_OPT_PROXY_URI, proxy_uri);
}

void CoapPacket::add_opt_proxy_uri( string proxy_uri )
{
	StringOption ins(COAP_OPT_PROXY_URI, proxy_uri);
	add_option(string_options_, ins);
}

void CoapPacket::set_opt_proxy_uri( list<string> proxy_uri )
{
	set_option(COAP_OPT_PROXY_URI, proxy_uri);
}

void CoapPacket::remove_opt_proxy_uri()
{
	remove_option( string_options_, COAP_OPT_PROXY_URI );
}

bool CoapPacket::opt_etag(uint8_t *etag, size_t length)
{
	return get_option(COAP_OPT_ETAG, etag, length);
}

void CoapPacket::set_opt_etag(uint8_t *etag, size_t length)
{
	OpaqueOption et(COAP_OPT_ETAG, etag, length);
	set_option(opaque_options_, et);
}

void CoapPacket::remove_opt_etag()
{
	remove_option(opaque_options_, COAP_OPT_ETAG);
}

bool CoapPacket::opt_uri_host( string &uri_host)
{
	return get_option(COAP_OPT_URI_HOST, uri_host);
}

void CoapPacket::set_opt_uri_host( string uri_host )
{
	StringOption uh(COAP_OPT_URI_HOST, uri_host);
	set_option(string_options_, uh);
}

bool CoapPacket::opt_location_path( list<string> &location_path )
{
	return get_option_all(COAP_OPT_LOCATION_PATH, location_path);
}

void CoapPacket::add_opt_location_path( string location_path )
{
	StringOption ins(COAP_OPT_LOCATION_PATH, location_path);
	add_option(string_options_, ins);
}

void CoapPacket::set_opt_location_path( list<string> location_path )
{
	set_option(COAP_OPT_LOCATION_PATH, location_path);
}

void CoapPacket::remove_opt_location_path()
{
	remove_option( string_options_, COAP_OPT_LOCATION_PATH );
}

bool CoapPacket::opt_uri_port(uint16_t & uri_port)
{
	uint32_t value = 0;
	if(get_option(COAP_OPT_URI_PORT, value))
	{
		uri_port = value & 0xffff;
		return true;
	}
	return false;
}

void CoapPacket::set_opt_uri_port( uint16_t uri_port )
{
	UintOption up(COAP_OPT_URI_PORT, uri_port);
	set_option(uint_options_, up);
}

bool CoapPacket::opt_location_query( list<string> &location_query )
{
	return get_option_all(COAP_OPT_LOCATION_QUERY, location_query);
}

void CoapPacket::add_opt_location_query( string location_query )
{
	StringOption ins(COAP_OPT_LOCATION_QUERY, location_query);
	add_option(string_options_, ins);
}

void CoapPacket::set_opt_location_query( list<string> location_query )
{
	set_option(COAP_OPT_LOCATION_QUERY, location_query);
}

void CoapPacket::remove_opt_location_query()
{
	remove_option( string_options_, COAP_OPT_LOCATION_QUERY );
}

bool CoapPacket::opt_uri_path( list<string> &uri_path )
{
	return get_option_all(COAP_OPT_URI_PATH, uri_path);
}

void CoapPacket::add_opt_uri_path( string uri_path )
{
	StringOption ins(COAP_OPT_URI_PATH, uri_path);
	add_option(string_options_, ins);
}

void CoapPacket::set_opt_uri_path( list<string> uri_path )
{
	set_option(COAP_OPT_URI_PATH, uri_path);
}

void CoapPacket::remove_opt_uri_path()
{
	remove_option( string_options_, COAP_OPT_URI_PATH );
}

bool CoapPacket::opt_token(uint8_t *token, size_t length)
{
	return get_option(COAP_OPT_TOKEN, token, length);
}

void CoapPacket::set_opt_token(uint8_t *token, size_t length)
{
	OpaqueOption tkn(COAP_OPT_TOKEN, token, length);
	set_option(opaque_options_, tkn);
}

void CoapPacket::remove_opt_token()
{
	remove_option(opaque_options_, COAP_OPT_TOKEN);
}

bool CoapPacket::opt_accept( list<uint16_t> &accept )
{
	list<UintOption> vals;
	bool status = get_option_all(uint_options_, COAP_OPT_ACCEPT, vals);
	if(status)
	{
		accept.clear();
		list<UintOption>::iterator it = vals.begin();
		uint16_t value = 0;
		for(; it != vals.end(); ++it)
		{
			value = ( *it ).value() && 0xffff;
			accept.push_back( value );
		}
		return true;
	}
	return false;
}

void CoapPacket::add_opt_accept( uint16_t accept )
{
	UintOption ins(COAP_OPT_ACCEPT, accept);
	add_option(uint_options_, ins);
}

void CoapPacket::set_opt_accept( list<uint16_t> accept )
{
	list<UintOption> opts;

	list<uint16_t>::iterator it = accept.begin();
	UintOption ins;
	for(; it != accept.end(); ++it)
	{
		ins.set( COAP_OPT_ACCEPT, ( *it ) );
		opts.push_back( ins );
	}
	set_option(uint_options_, opts);
}

void CoapPacket::remove_opt_accept ()
{
	remove_option( uint_options_, COAP_OPT_ACCEPT );
}

void CoapPacket::remove_opt_accept ( uint16_t accept )
{
	list<UintOption>::iterator it = uint_options_.begin();
	while(it != uint_options_.end())
	{
		if( ( *it ).option_number() == COAP_OPT_ACCEPT && ( *it ).value() == accept )
		{
			it = uint_options_.erase(it);
			continue;
		}
		++it;
	}
}

bool CoapPacket::opt_if_match( list<OpaqueOption> &if_match )
{
	return get_option_all(opaque_options_, COAP_OPT_IF_MATCH, if_match);
}

void CoapPacket::add_opt_if_match( uint8_t *value, size_t length )
{
	OpaqueOption if_match(COAP_OPT_IF_MATCH, value, length);
	add_option(opaque_options_, if_match);
}

void CoapPacket::remove_opt_if_match( OpaqueOption if_match )
{
	list<OpaqueOption>::iterator it = opaque_options_.begin();
	while(it != opaque_options_.end())
	{
		if( ( *it ).option_number() == COAP_OPT_IF_MATCH && ( *it ) == if_match )
		{
			it = opaque_options_.erase(it);
			continue;
		}
		++it;
	}
}

bool CoapPacket::opt_uri_query( list<string> &uri_query )
{
	return get_option_all(COAP_OPT_URI_QUERY, uri_query);
}

void CoapPacket::add_opt_uri_query( string uri_query )
{
	StringOption ins(COAP_OPT_URI_QUERY, uri_query);
	add_option(string_options_, ins);
}

void CoapPacket::set_opt_uri_query( list<string> uri_query )
{
	set_option(COAP_OPT_URI_QUERY, uri_query);
}

void CoapPacket::remove_opt_uri_query()
{
	remove_option( string_options_, COAP_OPT_URI_QUERY );
}

