
// Standard library:
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <signal.h>

// RabbitMQ
#include "rabbitmq/parameters.h"
#include "rabbitmq/connection.h"
#include "rabbitmq/channel.h"
#include "rabbitmq/management.h"
#include "rabbitmq/rabbit_mgr.h"

using namespace std;
using namespace rabbitmq;


int main ()
{
   const string   HOST          ("caerabbitmq.in2p3.fr");

   const uint16_t ADM_PORT      (15671);
   const string   ADM_VHOST     ("/");
   const string   ADM_LOGIN     ("supernemo_adm");
   const string   ADM_PASSWD    ("supernemo_adm");

   const uint16_t USR_PORT      (5671);
   const string   USR_VHOST     ("/PERM_Tests");
   const string   USR_LOGIN     ("ex_service");
   const string   USR_PASSWD    ("ex_service");

   const string   EXCHANGE      ("exchange.service");
   const string   TOPIC         ("#.perms.#");

   clog << "\nEXCHANGE SERVICE PERMS\n" ;
   bool           ok;
   error_response error;
   permissions    vh_user_perms;
   rabbit_mgr     mgr (HOST, ADM_PORT, ADM_LOGIN, ADM_PASSWD);

   clog << "\n== add vhost " << USR_VHOST << " ==\n" << endl;
   ok = mgr.add_vhost (USR_VHOST, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== exchange declare " << EXCHANGE << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.exchange_declare (EXCHANGE, USR_VHOST, "topic", true, false, false, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== add user " << USR_LOGIN << " ==" << endl;
   ok = mgr.add_user (USR_LOGIN, USR_PASSWD, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }
   vh_user_perms.user  = USR_LOGIN;
   vh_user_perms.vhost = USR_VHOST;
   vh_user_perms.vire_add_exchange_service_perms (EXCHANGE);
   clog << "\n== set permissions " << vh_user_perms.configure << " " << vh_user_perms.write << " " << vh_user_perms.read << " to "  << USR_LOGIN << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.set_permissions (vh_user_perms, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== user " << USR_LOGIN << " permissions on " << USR_VHOST << " ==" << endl;
   ok = mgr.user_permissions (USR_LOGIN, USR_VHOST, vh_user_perms, error);
   if (ok) {
      clog << "  " << vh_user_perms.configure << "  " << vh_user_perms.write << "  " << vh_user_perms.read << endl;
   } else {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////

   connection_parameters c_par;
   c_par.host   = HOST;
   c_par.port   = USR_PORT;
   c_par.vhost  = USR_VHOST;
   c_par.login  = USR_LOGIN;
   c_par.passwd = USR_PASSWD;
   connection       con (c_par);
   string           routing_key;
   string           message;
   string           request;
   uint64_t         delivery;
   basic_properties prop_in;
   basic_properties prop_out;
   channel &  chan = con.grab_channel ();
   queue_parameters q_par;
   q_par.name      = "";                                                 // q_par = "" -> rabbit will choose one for us
   q_par.exclusive = true;
   chan.queue_declare (q_par);
   clog <<  "queue name = " << q_par.name << endl;
   clog <<  "queue bind" << endl;
   chan.queue_bind    (q_par.name, EXCHANGE, TOPIC);                // q_par.name <- choosen by rabbit
   chan.basic_qos     (1);
   chan.basic_consume (q_par.name);
   clog << " [x] Awaiting RPC requests. Press return to exit." << endl;

   pid_t  service_pid = fork ();
   if (service_pid == 0) {
      while (1) {
         chan.consume_message (request, routing_key, prop_in, delivery);
         if (not prop_in.has_correlation_id () or not prop_in.has_reply_to ()) {
            chan.basic_ack (delivery);
            continue;
         }
         prop_out.set_correlation_id (prop_in.get_correlation_id ());
         chan.basic_publish          ("", prop_in.get_reply_to (), request, prop_out);
         chan.basic_ack              (delivery);
         clog << " [.] received request to " << routing_key << endl;
      }
   } else {
      cin.ignore ();
      kill (service_pid, SIGKILL);
   }


   /////////////////////////////////////////////////////////////////////////////////////////////////////////


   clog << "\n== delete user " << USR_LOGIN << " ==" << endl;
   ok = mgr.delete_user (USR_LOGIN, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== delete exchange " << EXCHANGE << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.delete_exchange (EXCHANGE, USR_VHOST, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== delete vhost " << USR_VHOST << " ==" << endl;
   ok = mgr.delete_vhost (USR_VHOST, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   return 0;
}

