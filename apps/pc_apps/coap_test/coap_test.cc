
#include <iostream>
#include <iomanip>

#include "external_interface/pc/pc_os_model.h"
#include "external_interface/pc/pc_timer.h"

#include "util/pstl/static_string.h"
#include "util/debugging.h"
#include "radio/coap/coap_packet_static.h"
#include "radio/coap/coap_service_static.h"

#include "udp4radio.h"
#include "ipv4_socket.h"
#include "touppercase.h"
#include "calculator.h"
#include "number_generator.h"
#include "obs_test.h"

using namespace wiselib;

typedef PCOsModel Os;
typedef UDP4Radio<Os> UPDRadio;

Os::Timer *timer;


int main(int argc, char** argv) {
	
	Os::Radio::self_pointer_t radio_;
	Os::Timer::self_pointer_t timer_;
	Os::Debug::self_pointer_t debug_;
	Os::Rand::self_pointer_t rand_;

	typedef wiselib::StaticString string_t;

	debug_ = new Os::Debug();
	timer_ = new Os::Timer();
	rand_ =  new Os::Rand();

	int port = COAP_STD_PORT;
	int coapsocket, acceptsocket, clilen, n;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	
	coapsocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(coapsocket < 0)
	{
		cerr << "Couldn't open Socket\n";
		exit(EXIT_FAILURE);
	}

	cout << "Created Socket " << port <<"\n";

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ( ::bind(coapsocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		int binderr = errno;
		cerr << "ERROR on binding, " << binderr << ": " << strerror(errno) << "\n";
		exit(EXIT_FAILURE);
	}

	cout << "Bound Socket " << port << "\n";

	listen(coapsocket,5);

	cout << "Listening on port " << port << "\n";

	UDP4Radio<Os> *udpradio = new UDP4Radio<Os>();

	typedef CoapServiceStatic<Os, UDP4Radio<Os>, Os::Timer, Os::Rand> coap_radio_t;

	coap_radio_t cradio_;

	cradio_.init( *udpradio,
			*timer_,
			*rand_);

	cradio_.enable_radio();

	ObsTest<Os, coap_radio_t, wiselib::StaticString> obs = ObsTest<Os, coap_radio_t, wiselib::StaticString>("oi", cradio_);

	ToUpperCase<coap_radio_t> uppercaser = ToUpperCase<coap_radio_t>();
	uppercaser.init(cradio_);

	Calculator<Os, coap_radio_t, wiselib::StaticString> calculator = Calculator<Os, coap_radio_t, wiselib::StaticString>();
	calculator.init(cradio_);

	NumberGenerator<Os, coap_radio_t, Os::Timer, wiselib::StaticString> generator = NumberGenerator<Os, coap_radio_t, Os::Timer, wiselib::StaticString>();
	generator.init(cradio_);

	wiselib::StaticString to_upper_path = wiselib::StaticString("touppercase");
	wiselib::StaticString calculator_path = wiselib::StaticString("calculator");
	wiselib::StaticString generator_path = wiselib::StaticString("generator");

	int to_upper_id = cradio_.reg_resource_callback< ToUpperCase<coap_radio_t>, &ToUpperCase<coap_radio_t>::receive_coap >( to_upper_path, &uppercaser );
	int calc_id = cradio_.reg_resource_callback< Calculator<Os, coap_radio_t, wiselib::StaticString>, &Calculator<Os, coap_radio_t, wiselib::StaticString>::receive_coap >( calculator_path, &calculator );
	int gen_id = cradio_.reg_resource_callback< NumberGenerator<Os, coap_radio_t, Os::Timer, wiselib::StaticString>, &NumberGenerator<Os, coap_radio_t, Os::Timer, wiselib::StaticString>::receive_coap >( generator_path, &generator );

	udpradio->set_socket( coapsocket );

	// Loop forever in a resource-efficient way
	// so timer events will actually occur
	while(true)
	{
		bzero(buffer,256);
		clilen = sizeof(cli_addr);
		n = recvfrom(coapsocket, buffer, 256, 0, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);

		if (n < 0)
		{
			// error case
			if( errno != EINTR )
			{
				cerr << "ERROR reading from socket, " << n << ", " << strerror(errno) << "\n";
				exit(EXIT_FAILURE);
			} else
			{
				//cerr << "ERROR signal interrupted reading from socket, " << n << ", " << strerror(errno) << "\n";
				continue;
			}
		}


		//debug_buffer<Os, 20, Os::Debug>(debug_, (UDP4Radio<Os>::block_data_t *) buffer, n);
		IPv4Socket sender_ipv4( cli_addr.sin_addr.s_addr, cli_addr.sin_port );
		size_t sender = udpradio->add_correspondent(sender_ipv4);
		udpradio->notify_receivers( sender, n, (UDP4Radio<Os>::block_data_t *) buffer );
	}
	
	return 0;
}
