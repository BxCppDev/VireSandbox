
// Standard library:
#include <iostream>
#include <unistd.h>
#include <signal.h>

//
#include "rabbitmq/parameters.h"
#include "rabbitmq/connection.h"
#include "rabbitmq/channel.h"


int main ()
{

   const std::string HOST              ("caerabbitmq.in2p3.fr");
   const uint16_t    PORT              (5671);
   const std::string VHOST             ("/test2/vire_cmslapp_service");
   const std::string LOGIN             ("hommet");
   const std::string PASSWD            ("**bunny**snemo**");
   const std::string SERVER_QUEUE_NAME ("VIRECMSServiceChannel");

   std::clog << "\nECHO RPC SERVER (Direct version)\n\n" ;
   rabbitmq::connection_parameters c_par;
   c_par.host   = HOST;
   c_par.port   = PORT;
   c_par.vhost  = VHOST;
   c_par.login  = LOGIN;
   c_par.passwd = PASSWD;
   rabbitmq::connection            con (c_par);
   rabbitmq::channel &             chan = con.grab_channel ();
   std::string                     routing_key;
   std::string                     request;
   uint64_t                        delivery;
   rabbitmq::basic_properties      prop_in;
   rabbitmq::basic_properties      prop_out;
   rabbitmq::queue_parameters      q_par;
   q_par.name      = SERVER_QUEUE_NAME;
//   q_par.exclusive = true;
   chan.queue_declare (q_par);
   chan.basic_qos     (1);
   chan.basic_consume (q_par.name);
   std::clog << " [x] Awaiting RPC requests. Press return to exit." << std::endl;

   pid_t  service_pid = fork ();
   if (service_pid == 0) {
      while (1) {
         chan.consume_message (request, routing_key, prop_in, delivery);
         std::clog << " REQ : " << request << std::endl;
         if (not prop_in.has_correlation_id () or not prop_in.has_reply_to ()) {
            chan.basic_ack (delivery);
            continue;
         }
         prop_out.set_correlation_id (prop_in.get_correlation_id ());
         chan.basic_publish          ("", prop_in.get_reply_to (), request, prop_out);
         chan.basic_ack              (delivery);
         std::clog << " [.] received request to " << routing_key << std::endl;
      }
   } else {
      std::cin.ignore ();
      kill (service_pid, SIGKILL);
   }
   return EXIT_SUCCESS;
}



