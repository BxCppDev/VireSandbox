
// Boost
#include <boost/program_options.hpp>

// Standard library:
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <memory>

//
#include "rabbitmq/parameters.h"
#include "rabbitmq/connection.h"
#include "rabbitmq/channel.h"
#include "rabbitmq/exception.h"

#include <vire/com/utils.h>
#include <vire/com/protobuf_encoding_driver.h>
#include <vire/utility/base_payload.h>
#include <vire/cms/connection_request.h>
#include <vire/cms/connection_success.h>
#include <vire/cms/resource_status_record.h>
#include <vire/message/message.h>
#include <vire/message/message_header.h>
#include <vire/message/message_body.h>
#include <vire/message/body_layout.h>

using namespace std;

struct app_params
{
   //  rabbit
   string   host;
   uint16_t port = 5671;
   string   vhost;
   string   login;
   string   passwd;
   string   queue_name;
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

         clog << "\nECHO RPC SERVER (Exchange version)\n\n" ;

         clog <<    "[info] host          = " << params.host             << endl;
         clog <<    "[info] port          = " << params.port             << endl;
         clog <<    "[info] vhost         = " << params.vhost            << endl;
         clog <<    "[info] login         = " << params.login            << endl;
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

         string                          routing_key;
         string                          request;
         uint64_t                        delivery;
         rabbitmq::basic_properties      prop_in;
         rabbitmq::basic_properties      prop_out;
         rabbitmq::exchange_parameters   x_par;
         rabbitmq::queue_parameters      q_par;
         rabbitmq::connection_parameters c_par;
         c_par.host      = params.host;
         c_par.port      = params.port;
         c_par.vhost     = params.vhost;
         c_par.login     = params.login;
         c_par.passwd    = params.passwd;
         q_par.name      = "";
         q_par.exclusive = true;

         rabbitmq::connection con (c_par);
         rabbitmq::channel &  chan = con.grab_channel ();
         q_par.exclusive = true;
         chan.queue_declare (q_par);
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
         chan.basic_qos     (1);
         chan.basic_consume (q_par.name);
         clog << " [x] Awaiting RPC requests. Press return to exit." << endl;

         std::string response;
         pid_t  service_pid = fork ();
         if (service_pid == 0) {
            while (1) {
               chan.consume_message (request, routing_key, prop_in, delivery);
               if (not prop_in.has_correlation_id () or not prop_in.has_reply_to ()) {
                  chan.basic_ack (delivery);
                  continue;
               }
               vire::com::protobuf_encoding_driver protoencoder;
               vire::message::message request_msg;
               vire::com::raw_message_type raw_request_msg;
               raw_request_msg.buffer = std::vector<char>(request.begin(), request.end());
               protoencoder.decode(raw_request_msg, request_msg);
               request_msg.tree_dump(std::cerr, "Request message:");

               vire::utility::const_payload_ptr_type request_payload
                 = request_msg.get_body().get_payload();

               std::shared_ptr<const vire::cms::connection_request> connRequestPtr
                 = std::dynamic_pointer_cast<const vire::cms::connection_request>(request_payload);
               if (connRequestPtr) {
                 const vire::cms::connection_request & connRequest = *connRequestPtr;
                 connRequest.tree_dump(std::cerr, "Connection Request:");

                 // Payload:
                 auto connSuccessPtr = std::make_shared<vire::cms::connection_success>();
                 auto & connSuccess = *connSuccessPtr;
                 for (const auto & resPath : connRequest.get_requested_resources()) {
                   vire::cms::resource_status_record record;
                   record.set_path(resPath);
                   record.set_timestamp(vire::time::now_utc());
                   record.set_missing();
                   connSuccess.add_resource_status_record(record);
                 }
                 connSuccess.tree_dump(std::cerr, "Built Connection Success:");

                 // Message:
                 vire::message::message resp_msg;

                 // Header:
                 vire::message::message_header     & resp_msg_header     = resp_msg.grab_header ();
                 vire::message::message_identifier   resp_msg_id           ("cmslapp", 42);
                 vire::utility::model_identifier     resp_body_layout_id;
                 resp_body_layout_id.set_name       (vire::message::body_layout::name());
                 resp_body_layout_id.set_version    (1);
                 resp_msg_header.set_body_layout_id (resp_body_layout_id);
                 resp_msg_header.set_message_id     (resp_msg_id);
                 resp_msg_header.set_timestamp      (vire::time::now ());
                 resp_msg_header.set_category       (vire::message::MESSAGE_RESPONSE);

                 // Body:
                 vire::message::message_body & resp_msg_body = resp_msg.grab_body ();
                 resp_msg_body.set_payload (connSuccessPtr);
                 
                 vire::com::protobuf_encoding_driver protoencoder;
                 vire::com::raw_message_type raw_response_msg;
                 protoencoder.encode(resp_msg, raw_response_msg);

                 response = std::string(raw_response_msg.buffer.begin(), raw_response_msg.buffer.end());
                 
                 prop_out.set_correlation_id (prop_in.get_correlation_id ());
                 chan.basic_publish          ("", prop_in.get_reply_to (), response, prop_out);
                 chan.basic_ack              (delivery);
               }                   
               clog << " [.] received request to " << routing_key << endl;
            }
         } else {
            cin.ignore ();
            kill (service_pid, SIGKILL);
         }
      }

   } catch (std::logic_error & error) {

      std::cerr << "[error] " << error.what() << std::endl;
      error_code = EXIT_FAILURE;


   } catch (rabbitmq::exception & error) {

      std::cerr << "[error] " << error.what() << std::endl;
      error_code = EXIT_FAILURE;

   } catch (...) {

      std::cerr << "[error] " << "Unexpected error!" << std::endl;
      error_code = EXIT_FAILURE;

   }

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

   ("exchange_name,x",
    po::value<std::string>(&params_.exchange_name)
    ->value_name("exchange_name"),
    "Set the exchange name. \n"
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




