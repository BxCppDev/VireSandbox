
#include <algorithm>

// Boost
#include <boost/program_options.hpp>

// RabbitMQ
#include "rabbitmq/rabbit_mgr.h"

using namespace std;
using namespace rabbitmq;


struct app_params
{
   string   host;
   uint16_t port = 15671;
   string   login;
   string   passwd;
   static void build_options (boost::program_options::options_description & opts_,
                              app_params                                  & params_);
   static void print_usage   (boost::program_options::options_description & opts_,
                              ostream                                     & out_);
};



int main (int argc_, char* argv_[])
{
   string   host;
   uint16_t port;
   string   login;
   string   passwd;
   int      error_code = 0;

   try {
      namespace po = boost::program_options;

      app_params                params;
      po::options_description   opts ("Allowed options ");
      app_params::build_options (opts, params);
      po::options_description   optPublic;
      optPublic.add             (opts);
      po::variables_map         vm;
      po::parsed_options        parsed = po::command_line_parser (argc_, argv_).options (optPublic).run ();
      po::store                 (parsed, vm);
      po::notify                (vm);

      if (vm.count ("help")) {
         if (vm ["help"].as<bool> ()) {
            app_params::print_usage (optPublic, std::cout);
            error_code = 1;
         }
      }
      if (error_code == 0) {
         if (params.host.empty ()) {
            params.host = "localhost";
         }
         if (params.login.empty ()) {
            params.login = "supernemo_adm";
         }
         if (params.passwd.empty ()) {
            app_params::print_usage (optPublic, std::cout);
            error_code = 1;
         }
         host   = params.host;
         port   = params.port;
         login  = params.login;
         passwd = params.passwd;
      }
   } catch (std::logic_error error) {
      std::cerr << "[error] " << error.what() << std::endl;
      return EXIT_FAILURE;
   } catch (...) {
      std::cerr << "[error] " << "Unexpected error!" << std::endl;
      return EXIT_FAILURE;
   }
   if (error_code != 0) {
      return EXIT_FAILURE;
   }

   string         user;
   string         vhost;
   string         exchange;
   permissions    perms;
   error_response error;
   bool           ok = true;

   rabbit_mgr mgr (host, port, login, passwd);

   clog << "USERS" << endl;
   rabbitmq::user::list users;
   rabbitmq::error_response err;
   mgr.list_users(users, err);
   user        = "vireserver";
   // if (std::find_if(users.begin(), users.end(), [&] (const rabbitmq::user & u) { return u.name == user; }) == users.end()) {
   //   if (ok)
   //   ok = mgr.add_user (user, user, error);
   // }
   ok = mgr.add_user (user, user, error);
  
   user        = "cmslapp";
   // if (std::find(users.begin(), users.end(), [&] (const rabbitmq::user & u) { return u.name == user; }) == users.end()) {
   //   if (ok)  ok = mgr.add_user (user, user, error);
   // }
   ok = mgr.add_user (user, user, error);
   ///////////////////////////////////////////////////////////////////////////////////////////
   ok = true;
   
   if (ok) clog << "GATE" << endl;
   vhost       = "/supernemo/demonstrator/cms/vire/clients/gate";
   exchange    = "gate.service";
   if (ok)  ok = mgr.add_vhost        (vhost, error);
   if (ok)  ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   user        = "supernemo_adm";
   if (ok)  ok = mgr.set_permissions     (user, vhost, ".*", ".*", ".*", error);
   user        = "vireserver";
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_service_perms (exchange);
   if (ok)  ok = mgr.set_permissions     (perms, error);

   ///////////////////////////////////////////////////////////////////////////////////////////

   if (ok) clog << "CONTROL" << endl;
   vhost       = "/supernemo/demonstrator/cms/vire/control";
   exchange    = "resource_request.service";
   if (ok)  ok = mgr.add_vhost        (vhost, error);
   if (ok)  ok = mgr.exchange_declare (exchange, vhost, "topic", true, false, false, error);
   user        = "supernemo_adm";
   if (ok)  ok = mgr.set_permissions     (user, vhost, ".*", ".*", ".*", error);
   user        = "vireserver";
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_service_perms (exchange);
   if (ok)  ok = mgr.set_permissions     (perms, error);

   ///////////////////////////////////////////////////////////////////////////////////////////

   if (ok) clog << "MONITORING" << endl;
   vhost       = "/supernemo/demonstrator/cms/vire/monitoring";
   const string Request_Ex = "resource_request.service";
   const string Log_Ex     = "log.event";
   const string Alarm_Ex   = "alarm.event";
   const string Pubsub_Ex  = "pubsub.event";
   if (ok)  ok = mgr.add_vhost        (vhost, error);
   if (ok)  ok = mgr.exchange_declare (Request_Ex, vhost, "topic", true, false, false, error);
   if (ok)  ok = mgr.exchange_declare (Log_Ex,     vhost, "topic", true, false, false, error);
   if (ok)  ok = mgr.exchange_declare (Alarm_Ex,   vhost, "topic", true, false, false, error);
   if (ok)  ok = mgr.exchange_declare (Pubsub_Ex,  vhost, "topic", true, false, false, error);
   if (ok)  ok = mgr.exchange_declare (Pubsub_Ex,  vhost, "topic", true, false, false, error);
   user        = "supernemo_adm";
   if (ok)  ok = mgr.set_permissions (user, vhost, ".*", ".*", ".*", error);
   user        = "vireserver";
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_service_perms        (Request_Ex);
   perms.vire_add_exchange_request_perms        (Request_Ex);
   perms.vire_add_exchange_event_producer_perms (Log_Ex);
   perms.vire_add_exchange_event_listener_perms (Log_Ex);
   perms.vire_add_exchange_event_producer_perms (Alarm_Ex);
   perms.vire_add_exchange_event_listener_perms (Alarm_Ex);
   perms.vire_add_exchange_event_producer_perms (Pubsub_Ex);
   perms.vire_add_exchange_event_listener_perms (Pubsub_Ex);
   if (ok)  ok = mgr.set_permissions            (perms, error);
   user        = "cmslapp";
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_service_perms        (Request_Ex);
   perms.vire_add_exchange_event_producer_perms (Log_Ex);
   perms.vire_add_exchange_event_producer_perms (Alarm_Ex);
   perms.vire_add_exchange_event_producer_perms (Pubsub_Ex);
   if (ok)  ok = mgr.set_permissions            (perms, error);

   ///////////////////////////////////////////////////////////////////////////////////////////

   if (ok) clog << "CMSLAPP SYSTEM" << endl;
   const string Vire_Service   = "vireserver.service";
   const string Vire_Event     = "vireserver.event";
   const string Subcon_Service = "subcontractor.service";
   const string Subcon_Event   = "subcontractor.event";
   vhost       = "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp";
   if (ok)  ok = mgr.add_vhost        (vhost, error);
   if (ok)  ok = mgr.exchange_declare (Vire_Service,   vhost, "topic", true, false, false, error);
   if (ok)  ok = mgr.exchange_declare (Vire_Event,     vhost, "topic", true, false, false, error);
   if (ok)  ok = mgr.exchange_declare (Subcon_Service, vhost, "topic", true, false, false, error);
   if (ok)  ok = mgr.exchange_declare (Subcon_Event,   vhost, "topic", true, false, false, error);
   if (ok) clog << "CMSLAPP SYSTEM exchanges declaration" << endl;
   user        = "supernemo_adm";
   if (ok)  ok = mgr.set_permissions  (user, vhost, ".*", ".*", ".*", error);
   //if (ok) clog << "CMSLAPP SYSTEM supernemo_adm set_permissions" << endl;
   user        = "vireserver";
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_service_perms        (Vire_Service);
   perms.vire_add_exchange_request_perms        (Subcon_Service);
   perms.vire_add_exchange_event_producer_perms (Vire_Event);
   perms.vire_add_exchange_event_listener_perms (Subcon_Event);
   if (ok)  ok = mgr.set_permissions            (perms, error);
   //if (ok) clog << "CMSLAPP SYSTEM vireserver set_permissions" << endl;
   user        = "cmslapp";
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_service_perms        (Subcon_Service);
   perms.vire_add_exchange_request_perms        (Vire_Service);
   perms.vire_add_exchange_event_producer_perms (Subcon_Event);
   perms.vire_add_exchange_event_listener_perms (Vire_Event);
   if (ok)  ok = mgr.set_permissions            (perms, error);
   //if (ok) clog << "CMSLAPP SYSTEM cmslapp set_permissions" << endl;

   ///////////////////////////////////////////////////////////////////////////////////////////

   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;

}


// static
void app_params::build_options(boost::program_options::options_description & opts_,
                               app_params                                  & params_)
{
   namespace po = boost::program_options;
   opts_.add_options()
   ("help,?",
    po::value<bool>()
    ->zero_tokens()
    ->default_value(false),
    "Produce help message")
   ("host,h",
    po::value<std::string>(&params_.host)
    ->value_name("host"),
    "Set the rabbitmq server host. \n"
    "Example :\n"
    " --host \"caerabbitmq.in2p3.fr\""
   )
   ("port,p",
    po::value<uint16_t>(&params_.port)
    ->value_name("port"),
    "Set the rabbitmq server port number. \n"
    "Example :\n"
    " --port 5671"
   )
   ("login,l",
    po::value<std::string>(&params_.login)
    ->value_name("login"),
    "Rabbitmq server management login. \n"
    "Example :\n"
    " --login \"supernemo_adm\""
   )
   ("password,w",
    po::value<std::string>(&params_.passwd)
    ->value_name("password"),
    "Manager password  *****  M A N D A T O R Y  *****\n"
    "Example :\n"
    " -w mgr_secret_word" 
   )
   ; //  end of options
   return;
}

void app_params::print_usage(boost::program_options::options_description & opts_,
                             ostream                                     & out_)
{
   static const string APP_NAME = "vire_domains";
   out_ << "\n" << APP_NAME << " -- " << endl;
   out_ << endl;
   out_ << "Usage : " << endl;
   out_ << std::endl;
   out_ << "  " << APP_NAME << " [OPTIONS] " << endl;
   out_ << opts_ << endl;
   return;
}




