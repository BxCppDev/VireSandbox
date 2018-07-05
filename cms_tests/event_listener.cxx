
// Standard library:
#include <iostream>

// Boost
#include <boost/program_options.hpp>

// BxProtobuftools
#include <bayeux/protobuftools/iofile.h>

// Vire
#include <vire/vire.h>
#include <vire/time/utils.h>
#include <vire/message/message.h>
#include <vire/message/message_header.h>
#include <vire/message/message_body.h>
#include <vire/message/body_layout.h>
#include <vire/utility/path.h>
#include <vire/cms/resource_exec.h>

// BxRabbitMQ
#include "rabbitmq/parameters.h"
#include "rabbitmq/connection.h"
#include "rabbitmq/channel.h"

using namespace std;

struct app_params
{
   //  rabbit
   string   host;
   uint16_t port = 5671;
   string   vhost;
   string   login;
   string   passwd;
   string   event_queue_name;
   string   exchange_name;
   string   topic_0;
   string   topic_1;
   string   topic_2;
   string   topic_3;
   int      nb_topics = 0;
   bool     exchange  = false;

   static void build_options (boost::program_options::options_description & opts_,
                              app_params                                  & params_);
   static void print_usage   (boost::program_options::options_description & opts_,
                              ostream                                     & out_);

};


int main (int argc_, char* argv_[])
{

   vire::initialize();
   int error_code = EXIT_SUCCESS;

   try {

      bool process = true;
      app_params params;
      namespace po = boost::program_options;
      po::options_description opts("Allowed options ");
      app_params::build_options(opts, params);
      // Aggregate options:
      po::options_description optPublic;
      optPublic.add(opts);
      po::variables_map vm;
      po::parsed_options parsed =
         po::command_line_parser(argc_, argv_)
         .options(optPublic)
         .run();
      po::store(parsed, vm);
      po::notify(vm);

      // Fetch the opts/args :
      if (vm.count("help")) {
         if (vm["help"].as<bool>()) {
            app_params::print_usage(optPublic, std::cout);
            error_code = 1;
            process = false;
         }
      }

      if (process) {
         if (params.host.empty ()) {
            params.host = "localhost";
         }
         if (params.vhost.empty ()) {
            params.vhost = "/supernemo/demonstrator/cms/vire/monitoring";
         }
         if (params.login.empty ()) {
            params.login = "vireserver";
         }
         if (params.passwd.empty ()) {
            params.passwd = "vireserver";
         }
         if (params.event_queue_name.empty ()) {
            params.event_queue_name = "";
         }
         if (params.exchange_name.empty ()) {
            params.exchange  = false;
            params.nb_topics = 0;
         } else {
            params.exchange  = true;
         }
         if (params.exchange) {
            if (params.topic_0.empty ()) {
               params.topic_0   = "#";
               params.nb_topics = 1;
            } else if (params.topic_1.empty ()) {
               params.nb_topics = 1;
            } else if (params.topic_2.empty ()) {
               params.nb_topics = 2;
            } else if (params.topic_3.empty ()) {
               params.nb_topics = 3;
            } else {
               params.nb_topics = 4;
            }
         }

         clog <<    "[info] host          = " << params.host             << endl;
         clog <<    "[info] port          = " << params.port             << endl;
         clog <<    "[info] vhost         = " << params.vhost            << endl;
         clog <<    "[info] login         = " << params.login            << endl;
         clog <<    "[info] event queue   = " << params.event_queue_name << endl;
         if (params.exchange) {
            clog << "[info] exchange name = " << params.exchange_name    << endl;
            clog << "[info] topic_0       = " << params.topic_0          << endl;
            if (params.nb_topics > 1) {
              clog << "[info] topic_1     = " << params.topic_1          << endl;
            }
            if (params.nb_topics > 2) {
              clog << "[info] topic_2     = " << params.topic_2          << endl;
            }
            if (params.nb_topics > 3) {
              clog << "[info] topic_3     = " << params.topic_3          << endl;
            }
         }

         std::string                     routing_key;
         rabbitmq::basic_properties      props;
         rabbitmq::exchange_parameters   x_par;
         rabbitmq::queue_parameters      q_par;
         rabbitmq::connection_parameters c_par;
         c_par.host      = params.host;
         c_par.port      = params.port;
         c_par.vhost     = params.vhost;
         c_par.login     = params.login;
         c_par.passwd    = params.passwd;
         q_par.name      = params.event_queue_name;
         q_par.exclusive = true;
         rabbitmq::connection con (c_par);
         rabbitmq::channel &  chan = con.grab_channel ();
         chan.queue_declare (q_par);
         clog << "AMQ-GEN EVENT QUEUE = " << q_par.name << endl;
         if (params.exchange) {
            x_par.name   = params.exchange_name;
            x_par.type   = "topic";
            // chan.exchange_declare (x_par);
            chan.queue_bind (q_par.name, x_par.name, params.topic_0);
            if (params.nb_topics > 1) {
               chan.queue_bind (q_par.name, x_par.name, params.topic_1);
            }
            if (params.nb_topics > 2) {
               chan.queue_bind (q_par.name, x_par.name, params.topic_2);
            }
            if (params.nb_topics > 3) {
               chan.queue_bind (q_par.name, x_par.name, params.topic_3);
            }
         }
         chan.basic_consume    (q_par.name, "", true);  // consumer_tag = "", no_ack = true
         clog << " [*] Waiting for events ... (ctrl-C to exit)" << endl;
         while (1) {
            std::string                     event;
            uint64_t                        delivery;
            clog << "call consume_message ..." << endl;
            try {
              chan.consume_message (event, routing_key, props, delivery);
              clog << "receiving something ..." << endl;
              clog << "                 event = '" << event << "'" << endl;
              clog << "           routing_key = '" << routing_key << "'" << endl;
 //            try {
//                stringstream event_protobuf;
//                event_protobuf << event;
//                // vire::message::message event_msg;
//                // protobuftools::load (event_protobuf, event_msg, 0);
//                // event_msg.tree_dump (std::clog, "Event : ");
//                break;
// //               clog << "EVENT RECEIVED : " << endl << event << endl;
              break;
            } catch (std::exception & error) {
              clog << "EVENT ERROR >>>>> " << endl << error.what() << endl << "<<<<<<" << endl;
            } catch (...) {
              clog << "EVENT ERROR >>>>> " << endl << event << endl << "<<<<<<" << endl;
            }
         }
      }

   } catch (std::logic_error & error) {

      std::cerr << "[error] " << error.what() << std::endl;
      error_code = EXIT_FAILURE;

   } catch (std::exception & error) {

      std::cerr << "[error] " << error.what() << std::endl;
      error_code = EXIT_FAILURE;

   } catch (...) {

      std::cerr << "[error] " << "Unexpected error!" << std::endl;
      error_code = EXIT_FAILURE;

   }

   vire::terminate();
   return error_code;

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

   ("vhost,v",
    po::value<std::string>(&params_.vhost)
    ->value_name("vhost"),
    "Set the virtual host. \n"
    "Example :\n"
    " --vhost \"/supernemo/demonstrator/cms/vire/monitoring\""
   )

   ("login,l",
    po::value<std::string>(&params_.login)
    ->value_name("login"),
    "Set the login name. \n"
    "Example :\n"
    " --login \"vireclient\""
   )

   ("password,w",
    po::value<std::string>(&params_.passwd)
    ->value_name("password"),
    "Set the password. \n"
    "Example :\n"
    " --passwd \"vireclient\""
   )

   ("event_queue,q",
    po::value<std::string>(&params_.event_queue_name)
    ->value_name("event_queue"),
    "Set the event queue name. \n"
    "Example :\n"
    " --event_queue \"my_event_queue_name\""
   )

   ("exchange_name,x",
    po::value<std::string>(&params_.exchange_name)
    ->value_name("exchange_name"),
    "Set the event exchange name. \n"
   )

   ("topic_0",
    po::value<std::string>(&params_.topic_0)
    ->value_name("topic_0"),
    "Bind queue and exchange with topic_0. \n"
   )

   ("topic_1",
    po::value<std::string>(&params_.topic_1)
    ->value_name("topic_1"),
    "Bind queue and exchange with topic_1. \n"
   )

   ("topic_2",
    po::value<std::string>(&params_.topic_2)
    ->value_name("topic_2"),
    "Bind queue and exchange with topic_2. \n"
   )

   ("topic_3",
    po::value<std::string>(&params_.topic_3)
    ->value_name("topic_3"),
    "Bind queue and exchange with topic_3. \n"
   )
   ; // end of options' description

   return;
}

void app_params::print_usage(boost::program_options::options_description & opts_,
                             ostream                                     & out_)
{
   static const string APP_NAME = "event_listener";
   out_ << "\n" << APP_NAME << " -- " << endl;
   out_ << endl;
   out_ << "Usage : " << endl;
   out_ << std::endl;
   out_ << "  " << APP_NAME << " [OPTIONS] " << endl;
   out_ << opts_ << endl;
   return;
}




