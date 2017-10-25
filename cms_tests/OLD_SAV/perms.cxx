
// Standard library:
#include <iostream>
#include <fstream>
#include <cstdlib>

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
   const string   USR_VHOST     ("/The_Mothers");
   const string   USR_LOGIN     ("Zappa");
   const string   USR_PASSWD    ("Freakout");

   const string   EXCHANGE_NAME ("Won_Ton_On");
   const string   QUEUE_NAME    ("Ya_Hozna");
   const string   AMQGEN        ("^amq\\.gen.*$");
   const string   AMQDEF        ("^amq\\.default$");
//   const string   USR_C_PERMS   (".*");
   const string   USR_C_PERMS   (AMQGEN);
//   const string   USR_W_PERMS   (".*");
   const string   USR_W_PERMS   (AMQGEN + "|^" + EXCHANGE_NAME + "$");
//   const string   USR_W_PERMS   ("^" + EXCHANGE_NAME + "$");
//   const string   USR_R_PERMS   (".*");
//   const string   USR_R_PERMS   (AMQGEN);
   const string   USR_R_PERMS   (AMQGEN + "|^" + EXCHANGE_NAME + "$");
   const string   TOPIC         ("one.size.fits.all");

   clog << "\nPERMS\n" ;
   bool                        ok;
   error_response    error;
   vhost::list       vhosts;
   exchange::list    exchanges;
   queue::list       queues;
   user::list        users;
   permissions       vh_user_perms;
   permissions::list user_perms;

   /////////////////////////////////////////////////////////////////////////////////////////////////////////

   rabbit_mgr        mgr (HOST, ADM_PORT, ADM_LOGIN, ADM_PASSWD);

   clog << "\n== add vhost " << USR_VHOST << " ==\n" << endl;
   ok = mgr.add_vhost (USR_VHOST, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== exchange declare " << EXCHANGE_NAME << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.exchange_declare (EXCHANGE_NAME, USR_VHOST, "topic", true, false, false, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== queue declare " << QUEUE_NAME << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.queue_declare (QUEUE_NAME, USR_VHOST, true, false, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== add user " << USR_LOGIN << " ==" << endl;
   ok = mgr.add_user (USR_LOGIN, USR_PASSWD, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== set permissions " << USR_C_PERMS << " " << USR_W_PERMS << " " << USR_R_PERMS << " to "  << USR_LOGIN << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.set_permissions (USR_LOGIN, USR_VHOST, USR_C_PERMS, USR_W_PERMS, USR_R_PERMS, error);
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

   clog <<  "user Zappa ..." << endl;
   connection_parameters c_par;
   c_par.host   = HOST;
   c_par.port   = USR_PORT;
   c_par.vhost  = USR_VHOST;
   c_par.login  = USR_LOGIN;
   c_par.passwd = USR_PASSWD;
   connection con (c_par);
   string routing_key;
   string message;
   basic_properties props;
   channel &  chan = con.grab_channel ();
   queue_parameters q_par;
   q_par.name      = "";                                                 // q_par = "" -> rabbit will choose one for us
   q_par.exclusive = true;
   chan.queue_declare (q_par);
   clog <<  "queue name = " << q_par.name << endl;
   clog <<  "queue bind" << endl;
   chan.queue_bind    (q_par.name, EXCHANGE_NAME, TOPIC);                // q_par.name <- choosen by rabbit
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

   clog << "\n== delete queue " << QUEUE_NAME << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.delete_queue (QUEUE_NAME, USR_VHOST, error);
   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
   }

   clog << "\n== delete exchange " << EXCHANGE_NAME << " on " << USR_VHOST << " ==" << endl;
   ok = mgr.delete_exchange (EXCHANGE_NAME, USR_VHOST, error);
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

