#ifndef UDP4RADIO_H_
#define UDP4RADIO_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include "ipv4_socket.h"

#include "util/pstl/vector_static.h"

namespace wiselib
{
	static const size_t NUM_CORRESPONDENTS = 50;

	template<typename OsModel_P>
	class UDP4Radio : public RadioBase<OsModel_P,
		size_t,
		typename OsModel_P::size_t,
		typename OsModel_P::block_data_t>
	{
	public:
		typedef OsModel_P OsModel;

		typedef UDP4Radio<OsModel> self_type;
		typedef self_type* self_pointer_t;

		typedef size_t node_id_t;
		typedef uint8_t block_data_t;
		typedef uint16_t message_id_t;

		enum SpecialNodeIds {
			BROADCAST_ADDRESS = 0xffffffff,
			NULL_NODE_ID      = 0
		};

		UDP4Radio() {
#ifdef DEBUG_COAPRADIO_PC
		cout << "UDP4Radio::UDP4Radio()";
#endif
			enabled = false;
			sock = 0;

			IPv4Socket IPv4Ssock = IPv4Socket();
			serv_addr = IPv4Ssock.operator int();
			corr_index = 1; // adress 0 seems to make trouble so we start at 1
			correspondents_.assign( NUM_CORRESPONDENTS, IPv4Socket() );
		}

		void set_socket( int socket )
		{
			sock = socket;
		}

		int enable_radio ()
		{
#ifdef DEBUG_COAPRADIO_PC
		cout << "UDP4Radio::enable_radio\n";
#endif

			enabled = true;
			return OsModel::SUCCESS;
		}

		int disable_radio ()
		{
#ifdef DEBUG_COAPRADIO_PC
		cout << "UDP4Radio::disable_radio";
#endif
			enabled = false;
			return OsModel::SUCCESS;
		}
		
		int send (node_id_t receiver_index, size_t len, block_data_t *data)
		{
			IPv4Socket receiver = correspondents_.at( receiver_index );
			sockaddr_in recv_sock;
			recv_sock.sin_family = AF_INET;
			recv_sock.sin_port = receiver.port();
			recv_sock.sin_addr.s_addr = receiver.addr();

			if(enabled)
			{
#ifdef DEBUG_COAPRADIO_PC
				cout << "UDP4Radio::send> sending " << len << " Bytes to "
										<< ((recv_sock.sin_addr.s_addr & 0xff000000) >> 24)
										<< "."
										<< ((recv_sock.sin_addr.s_addr & 0x00ff0000) >> 16)
										<< "."
										<< ((recv_sock.sin_addr.s_addr & 0x0000ff00) >> 8)
										<< "."
										<< (recv_sock.sin_addr.s_addr & 0x000000ff)
										<< "\n";
#endif
				sendto( sock, data, len, 0, (struct sockaddr *) &recv_sock, sizeof( struct sockaddr_in ));
				return OsModel::SUCCESS;
			}
			return OsModel::ERR_UNSPEC;
		}

		node_id_t id ()
		{
			return serv_addr;
		}

		node_id_t add_correspondent( IPv4Socket corr )
		{
			node_id_t index = find_correspondent(corr);
			if ( index == (NUM_CORRESPONDENTS+1) )
			{
				// not yet in list
				index = corr_index;
				correspondents_.at(index) = corr;
				++corr_index;
				if(corr_index == NUM_CORRESPONDENTS)
					corr_index = 1;
			}
			return index;
		}

	private:
		bool enabled;
		int sock;
		node_id_t serv_addr;

		size_t corr_index;
		vector_static<OsModel, IPv4Socket, NUM_CORRESPONDENTS> correspondents_;

		size_t find_correspondent( IPv4Socket corr )
		{
			for ( size_t i = 1; i < correspondents_.size(); i++ )
			{
				IPv4Socket curr = correspondents_.at(i);
				if ( curr == corr )
				{
					return i;
				}
			}
			return NUM_CORRESPONDENTS +1;
		}

	};
}

#endif /* UDP4RADIO_H_ */
