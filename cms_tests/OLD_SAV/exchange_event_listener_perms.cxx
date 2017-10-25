
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

void my_callback (string & msg_, string & rkey_, basic_properties & props_) {
   clog << " [x]  " << rkey_ << "  " << msg_ << endl;
}

int main ()
{
   const string   HOST          ("caerabbitmq.in2p3.fr");

   const uint16_t ADM_PORT      (15671);
   const string   ADM_VHOST     ("/");
   const string   ADM_LOGIN     ("supernemo_adm");
   const string   ADM_PASSWD    ("supernemo_adm");

   const uint16_t USR_PORT      (5671);
   const string   USR_VHOST     ("/PERM_Tests");
   const string   USR_LOGIN     ("ex_evt_listener");
   const string   USR_PASSWD    ("ex_evt_listener");

   const string   EXCHANGE      ("exchange.event");
   const string   TOPIC         ("#.perms.#");

   clog << "\nEXCHANGE EVENT LISTENER PERMS\n" ;
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
   vh_user_perms.vire_add_exchange_event_listener_perms (EXCHANGE);
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
   connection con (c_par);
   channel &  chan = con.grab_channel ();
   queue_parameters q_par;
   q_par.name      = "";                                                 // q_par = "" -> rabbit will choose one for us
   q_par.exclusive = true;
   chan.queue_declare (q_par);
   chan.queue_bind    (q_par.name, EXCHANGE, TOPIC);                     // q_par.name <- choosen by rabbit
   chan.basic_consume (q_par.name, "", true);                            // consumer_tag = "", no_ack = true
   clog << " [*] Waiting for messages. Press Return to exit." << endl;
   chan.start_consuming (&my_callback);
   cin.ignore ();
   chan.stop_consuming ();

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

