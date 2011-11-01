/*
 * Simple Wiselib Example
 */
#include "external_interface/external_interface.h"
#include "algorithms/routing/tree/tree_routing.h"

#include "radio/coap/coap_packet.h"


typedef wiselib::OSMODEL Os;

class ExampleApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         debug_->debug( "Example Application booting!\n" );

         radio_->reg_recv_callback<ExampleApplication,
                                   &ExampleApplication::receive_radio_message>( this );
         timer_->set_timer<ExampleApplication,
                           &ExampleApplication::broadcast_loop>( 5000, this, 0 );

         wiselib::CoapPacket<Os> testpacket();
      }
      // --------------------------------------------------------------------
      void broadcast_loop( void* )
      {
         debug_->debug( "broadcasting message at %x \n", radio_->id() );
         Os::Radio::block_data_t message[] = "hello world!\0";
         radio_->send( Os::Radio::BROADCAST_ADDRESS, sizeof(message), message );

         // following can be used for periodic messages to sink
         timer_->set_timer<ExampleApplication,
                           &ExampleApplication::broadcast_loop>( 5000, this, 0 );
      }
      // --------------------------------------------------------------------
      void receive_radio_message( Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *buf )
      {
         debug_->debug( "received msg at %x from %x\n", radio_->id(), from );
         debug_->debug( "message is %s\n", buf );
      }
   private:
      Os::Radio::self_pointer_t radio_;
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}