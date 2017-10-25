

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

   string         vhost;
   string         server_service;
   string         server_event;
   string         user;
   permissions    perms;
   error_response error;
   bool           ok = true;

   rabbit_mgr mgr (host, port, login, passwd);

   clog << "ADD TMP USER wxCVbn" << endl;
   user        = "wxCVbn";
   if (ok) ok  = mgr.add_user (user, user, error);

   clog << "CONTROL add user's perms" << endl;
   perms.clear ();
   perms.user  = user;
   perms.vhost = "/supernemo/demonstrator/cms/vire/control";
   perms.vire_add_exchange_request_perms ("resource_request.service");
   if (ok) ok  = mgr.set_permissions     (perms, error);

   clog << "MONITORING add user's perms" << endl;
   perms.clear ();
   perms.user  = user;
   perms.vhost = "/supernemo/demonstrator/cms/vire/monitoring";
   perms.vire_add_exchange_request_perms        ("resource_request.service");
   perms.vire_add_exchange_event_listener_perms ("alarm.event");
   perms.vire_add_exchange_event_listener_perms ("log.event");
   perms.vire_add_exchange_event_listener_perms ("pubsub.event");
   if (ok) ok  = mgr.set_permissions       (perms, error);

   clog << "ADD TMP CLIENT SYSTEM & perms" << endl;
   vhost          = "/supernemo/demonstrator/cms/vire/clients/system/" + user;
   if (ok) ok     = mgr.add_vhost        (vhost, error);
   server_service = "vireserver.service";
   server_event   = "vireserver.event";
   if (ok) ok     = mgr.exchange_declare (server_service, vhost, "topic", true, false, false, error);
   if (ok) ok     = mgr.exchange_declare (server_event,   vhost, "topic", true, false, false, error);
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_request_perms        (server_service);
   perms.vire_add_exchange_event_listener_perms (server_event);
   if (ok) ok  = mgr.set_permissions            (perms, error);
   user        = "vireserver";
   perms.clear ();
   perms.user  = user;
   perms.vhost = vhost;
   perms.vire_add_exchange_service_perms         (server_service);
   perms.vire_add_exchange_event_producer_perms  (server_event);
   if (ok) ok  = mgr.set_permissions             (perms, error);
   user        = "supernemo_adm";
   if (ok) ok  = mgr.set_permissions  (user, vhost, ".*", ".*", ".*", error);

   if (!ok) {
      cerr << " Error : " << error.error << "  " << error.reason << endl;
      return 1;
   }
   return 0;

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
