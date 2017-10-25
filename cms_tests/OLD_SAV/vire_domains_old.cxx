

// RabbitMQ
#include "rabbitmq/rabbit_mgr.h"

using namespace std;
using namespace rabbitmq;


string keep_dots (const string & dotword)
{
   //  "bob.robert.boby" => "bob\\.robert\\.boby"
   string word (dotword);
   size_t pos = 0;
   while ((pos = word.find (".", pos)) != string::npos) {
      word.replace (pos, 1, "\\\\.");
      pos = pos + 3;
   }
   //return word;
   return dotword;
}


int main ()
{
   const string   HOST        ("caerabbitmq.in2p3.fr");
   const uint16_t ADM_PORT    (15671);
   const string   ADM_LOGIN   ("supernemo_adm");
   const string   ADM_PASSWD  ("supernemo_adm");
   const string   AMQGEN      ("^amq.gen*");
   const string   AMQDEF      ("^amq.default$");

   string         vhost;
   string         conf_perm;
   string         writ_perm;
   string         read_perm;
   string         exchange;
   string         user;
   error_response error;
   bool           ok = true;

   rabbit_mgr mgr (HOST, ADM_PORT, ADM_LOGIN, ADM_PASSWD);

   clog << "USERS" << endl;

//   user       = "supernemo_adm";                   //  'supernemo_adm' already created with
//   if (ok) ok = mgr.add_user (user, user, error);  //  > sudo rabbitmqctl add_user supernemo_adm supernemo_adm
                                                     //  > sudo rabbitmqctl set_user_tags supernemo_adm administrator management
   user       = "vireserver";
   if (ok) ok = mgr.add_user (user, user, error);
//   user       = "vireclient";
//   if (ok) ok = mgr.add_user (user, user, error);
   user       = "cmslapp";
   if (ok) ok = mgr.add_user (user, user, error);

   //  SOPHIE  ////////////////////////////////////
   user       = "vire";
   if (ok) ok = mgr.add_user (user, user, error);
   user       = "client";
   if (ok) ok = mgr.add_user (user, user, error);
   ////////////////////////////////////////////////

   clog << "GATE" << endl;
   vhost      = "/supernemo/demonstrator/cms/vire/clients/gate";
   exchange   = "gate.service";
   if (ok) ok = mgr.add_vhost        (vhost, error);
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   user       = "vireserver";
   conf_perm  = AMQGEN;
   writ_perm  = AMQGEN + "|" + AMQDEF;
   read_perm  = AMQGEN + "|^" + keep_dots (exchange) + "$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
//   user       = "vireclient";
//   conf_perm  = AMQGEN;
//   writ_perm  = "^" + keep_dots (exchange) + "$";
//   read_perm  = AMQGEN;
//   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "supernemo_adm";
   conf_perm  = ".*";
   writ_perm  = ".*";
   read_perm  = ".*";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "hommet";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);

   clog << "CONTROL" << endl;
   vhost      = "/supernemo/demonstrator/cms/vire/control";
   exchange   = "resource_request.service";
   if (ok) ok = mgr.add_vhost        (vhost, error);
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   user       = "vireserver";
   conf_perm  = AMQGEN;
   writ_perm  = AMQGEN + "|" + AMQDEF;
   read_perm  = AMQGEN + "|^" + keep_dots (exchange) + "$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
//   user       = "vireclient";
//   conf_perm  = AMQGEN;
//   writ_perm  = "^" + keep_dots (exchange) + "$";
//   read_perm  = AMQGEN;
//   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "supernemo_adm";
   conf_perm  = ".*";
   writ_perm  = ".*";
   read_perm  = ".*";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "hommet";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);

   clog << "MONITORING" << endl;
   vhost      = "/supernemo/demonstrator/cms/vire/monitoring";
   if (ok) ok = mgr.add_vhost        (vhost, error);
   exchange   = "resource_request.service";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   exchange   = "alarm.event";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   exchange   = "log.event";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   exchange   = "pubsub.event";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   user       = "vireserver";
   conf_perm  = AMQGEN;
   //writ_perm  = AMQGEN + "|^resource_request\\.service$|^alarm\\.event$|^log\\.event$|^pubsub\\.event$";
   //read_perm  = AMQGEN + "|^resource_request\\.service$|^alarm\\.event$|^log\\.event$|^pubsub\\.event$";
   writ_perm  = AMQGEN + "|^resource_request.service$|^alarm.event$|^log.event$|^pubsub.event$" + "|" + AMQDEF;
   read_perm  = AMQGEN + "|^resource_request.service$|^alarm.event$|^log.event$|^pubsub.event$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
//   user       = "vireclient";
//   conf_perm  = AMQGEN;
//   //writ_perm  = AMQGEN + "|^resource_request\\.service$";
//   //read_perm  = AMQGEN + "|^alarm\\.event$|^log\\.event$|^pubsub\\.event$";
//   writ_perm  = AMQGEN + "|^resource_request.service$";
//   read_perm  = AMQGEN + "|^alarm.event$|^log.event$|^pubsub.event$";
//   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "cmslapp";
   conf_perm  = AMQGEN;
   //writ_perm  = AMQGEN + "|^alarm\\.event$|^log\\.event$|^pubsub\\.event$";
   //read_perm  = AMQGEN + "|^resource_request\\.service$";
   // SOPHIE //////////////////////////////////////////
   //writ_perm  = AMQGEN + "|^alarm.event$|^log.event$|^pubsub.event$" + "|" + AMQDEF;
   writ_perm  = AMQGEN + "|^alarm.event$|^log.event$|^pubsub.event$" + "|" + AMQDEF + "|^AsynchroneChannel$";
   //////////////////////////////////////////
   read_perm  = AMQGEN + "|^resource_request.service$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "supernemo_adm";
   conf_perm  = ".*";
   writ_perm  = ".*";
   read_perm  = ".*";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
//   user       = "hommet";
//   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
//   user       = "lieunard";
//   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   //  SOPHIE  ////////////////////////////////////
   user       = "client";
   conf_perm  = AMQGEN + "|^AsynchroneChannel$";
   //writ_perm  = AMQGEN + "|^resource_request\\.service$";
   //read_perm  = AMQGEN + "|^alarm\\.event$|^log\\.event$|^pubsub\\.event$";
   writ_perm  = AMQGEN + "|^resource_request.service$";
   read_perm  = AMQGEN + "|^alarm.event$|^log.event$|^pubsub.event$" + "|^AsynchroneChannel$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "vire";
   conf_perm  = AMQGEN + "|^AsynchroneChannel$";
   //writ_perm  = AMQGEN + "|^resource_request\\.service$|^alarm\\.event$|^log\\.event$|^pubsub\\.event$";
   //read_perm  = AMQGEN + "|^resource_request\\.service$|^alarm\\.event$|^log\\.event$|^pubsub\\.event$";
   writ_perm  = AMQGEN + "|^resource_request.service$|^alarm.event$|^log.event$|^pubsub.event$";
   read_perm  = AMQGEN + "|^resource_request.service$|^alarm.event$|^log.event$|^pubsub.event$" + "|^AsynchroneChannel$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);

   clog << "CMSLAPP SYSTEM" << endl;
   vhost      = "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp";
   if (ok) ok = mgr.add_vhost        (vhost, error);
   exchange   = "vireserver.event";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   exchange   = "vireserver.service";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   exchange   = "subcontractor.event";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   exchange   = "subcontractor.service";
   if (ok) ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   user       = "vireserver";
   conf_perm  = AMQGEN;
   //writ_perm  = AMQGEN + "|^vireserver\\.event$|^subcontractor\\.service$";
   //read_perm  = AMQGEN + "|^vireserver\\.service$|^subcontractor\\.event$";
   writ_perm  = AMQGEN + "|^vireserver.event$|^subcontractor.service$" + "|" + AMQDEF;
   read_perm  = AMQGEN + "|^vireserver.service$|^subcontractor.event$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "cmslapp";
   conf_perm  = AMQGEN;
   writ_perm  = AMQGEN + "|^vireserver\\.service$|^subcontractor\\.event$";
   read_perm  = AMQGEN + "|^vire_server\\.event$|^subcontractor\\.service$";
   //
   // SOPHIE /////////////////////////////////
   //writ_perm  = AMQGEN + "|^vireserver.service$|^subcontractor.event$" + "|" + AMQDEF;
   writ_perm  = AMQGEN + "|^vireserver.service$|^subcontractor.event$" + "|" + AMQDEF + "|^AsynchroneChannel$";
   ////////////////////////////////////////////
   read_perm  = AMQGEN + "|^vire_server.event$|^subcontractor.service$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   user       = "supernemo_adm";
   conf_perm  = ".*";
   writ_perm  = ".*";
   read_perm  = ".*";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
//   user       = "hommet";
//   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
//   user       = "lieunard";
//   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);
   //  SOPHIE ///////////////////////////////////////////////////////////////
   user       = "vire";
   conf_perm  = AMQGEN + "|^AsynchroneChannel$";
   //writ_perm  = AMQGEN + "|^vireserver\\.event$|^subcontractor\\.service$";
   //read_perm  = AMQGEN + "|^vireserver\\.service$|^subcontractor\\.event$";
   writ_perm  = AMQGEN + "|^vireserver.event$|^subcontractor.service$";
   read_perm  = AMQGEN + "|^vireserver.service$|^subcontractor.event$" + "|^AsynchroneChannel$";
   if (ok) ok = mgr.set_permissions  (user, vhost, conf_perm, writ_perm, read_perm, error);

   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
      return 1;
   }
   return 0;

}

