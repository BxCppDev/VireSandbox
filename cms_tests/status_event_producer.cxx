
// Standard library:
#include <iostream>
#include <fstream>
#include <cstdlib>

// Boost
#include <boost/program_options.hpp>

// BxProtobuftools:
#include <bayeux/protobuftools/iofile.h>

// Vire:
#include <vire/vire.h>
#include <vire/time/utils.h>
#include <vire/message/message.h>
#include <vire/message/message_header.h>
#include <vire/message/message_body.h>
#include <vire/message/body_layout.h>
#include <vire/utility/path.h>
#include <vire/cms/resource_status_record.h>

// RabbitMQ
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
   string   exchange_name;

   //  event
   string   mode;
   string   setup;
   string   leaf;
   string   dirs;
   bool     failed   = false;
   bool     missing  = false;
   bool     disabled = false;
   bool     pending  = false;

   static void build_options (boost::program_options::options_description & opts_,
                              app_params                                  & params_);
   static void print_usage   (boost::program_options::options_description & opts_,
                              ostream                                     & out_);

};


int main (int argc_, char* argv_[])
{
   vire::initialize();
   int error_code = EXIT_SUCCESS;

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
   int            nb_evt_args = 0;

   // Fetch the opts/args :
   if (vm.count("help")) {
      if (vm["help"].as<bool>()) {
         app_params::print_usage(optPublic, std::cout);
         error_code = 1;
         process = false;
      }
   }
   if (vm.count("pending")) {
      if (vm["pending"].as<bool>()) {
         params.pending = true;
      }
   }
   if (vm.count("missing")) {
      if (vm["missing"].as<bool>()) {
         params.missing = true;
      }
   }
   if (vm.count("failed")) {
      if (vm["failed"].as<bool>()) {
         params.failed = true;
      }
   }
   if (vm.count("disabled")) {
      if (vm["disabled"].as<bool>()) {
         params.disabled = true;
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
         params.exchange_name = "log.event";
      }
      if (params.setup.empty()) {
         params.setup = "SuperNEMO";
      }
      if (params.dirs.empty()) {
         params.dirs = "/Demonstrator/CMS/Coil/PS_I0/Monitoring/Voltage";
         //params.dirs = "/Demonstrator/CMS/Coil/PS_I0/Control/Voltage";
         //params.dirs = "/Demonstrator/CMS/Coil/PS_I1";   //  mos simulé (exec synchone  =>  pas le même comportement)
      }
      if (params.leaf.empty()) {
         params.leaf = "__dp_read__";
         //params.leaf = "__dp_write__";
         //params.leaf = "StartCycling";
      }

      string resource_path = vire::utility::path::build      (params.setup, params.dirs, params.leaf);
      string routing_key   = vire::utility::path::to_address (resource_path);

      clog <<    endl;
      clog <<    "--------------------------------------------------------------------------------------" << endl;

      clog <<    "[info] host              " << params.host   << endl;
      clog <<    "[info] port              " << params.port   << endl;
      clog <<    "[info] vhost             " << params.vhost  << endl;
      clog <<    "[info] login             " << params.login  << endl;
      clog <<    "[info] event box         " << params.exchange_name      << endl;
      clog <<    "[info] resource path     " << resource_path << endl;
      clog <<    "[info] routing key       " << routing_key << endl;

      // Message:
      vire::message::message evt_msg;

      // Header:
      vire::message::message_header     & evt_msg_header     = evt_msg.grab_header ();
      vire::message::message_identifier   evt_msg_id           ("vire.server", 42);
      vire::utility::model_identifier     evt_body_layout_id;
      evt_body_layout_id.set_name       (vire::message::body_layout::name());
      evt_body_layout_id.set_version    (1);
      evt_msg_header.set_body_layout_id (evt_body_layout_id);
      evt_msg_header.set_message_id     (evt_msg_id);
      evt_msg_header.set_timestamp      (vire::time::now ());
      evt_msg_header.set_category       (vire::message::MESSAGE_EVENT);
//         evt_msg_header.add_metadata       ("user_correlation_id", routing_key);

      // Payload:
      vire::cms::resource_status_record rsr;
      rsr.set_path      (resource_path);
      rsr.set_timestamp (vire::time::now ());
      rsr.unset_all     ();
      if (params.failed)   rsr.set_failed   ();
      if (params.pending)  rsr.set_pending  ();
      if (params.missing)  rsr.set_missing  ();
      if (params.disabled) rsr.set_disabled ();
      std::clog << std::endl;
      rsr.tree_dump (std::clog, "Status event produce : ");
      std::clog << std::endl;

      // Body:
      vire::message::message_body & evt_msg_body = evt_msg.grab_body ();
      evt_msg_body.set_payload (rsr);
      vire::utility::model_identifier payload_type_id1 = evt_msg_body.get_payload_type_id ();
      vire::utility::model_identifier payload_type_id2;
      payload_type_id2.set (payload_type_id1.get_name (), 1);
      evt_msg_body.set_payload_type_id (payload_type_id2);

      evt_msg.tree_dump(std::clog, "Evt. Message: ");
      std::clog << std::endl;

      // Generate protobufized message:
      std::ostringstream evt_protobuf;
      protobuftools::store (evt_protobuf, evt_msg, 0);

      // Rabbit connection
      const bool                      pub_confirm  = true;
      rabbitmq::connection_parameters c_par;
                                      c_par.host   = params.host;
                                      c_par.port   = params.port;
                                      c_par.vhost  = params.vhost;
                                      c_par.login  = params.login;
                                      c_par.passwd = params.passwd;
      rabbitmq::connection            con (c_par, pub_confirm);
      rabbitmq::channel &             chan         = con.grab_channel ();

      uint64_t                        delivery;
      rabbitmq::basic_properties      prop_out;
      rabbitmq::basic_properties      prop_in;
      rabbitmq::exchange_parameters   x_par;
      x_par.name = params.exchange_name;
      x_par.type = "topic";
      chan.basic_publish (x_par.name, routing_key, evt_protobuf.str (), prop_out);
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
    " --vhost \"/test2/vire_cmslapp_service\""
   )

   ("login,l",
    po::value<std::string>(&params_.login)
    ->value_name("login"),
    "Set the login name. \n"
    "Example :\n"
    " --login \"vire\""
   )

   ("password,w",
    po::value<std::string>(&params_.passwd)
    ->value_name("passwd"),
    "Set the password. \n"
    "Example :\n"
    " --passwd \"vire\""
   )

   ("exchange_name",
    po::value<std::string>(&params_.exchange_name)
    ->value_name("name"),
    "Set request passing by exchange. \n"
    "Example :\n"
    " --exchange_name PublicExchangeName"
   )

   ("dirs,d",
    po::value<std::string>(&params_.dirs)
    ->value_name("name"),
    "Set the resource dirs path. \n"
    "Example :\n"
    " --dirs \"/Demonstrator/CMS/Coil/Control/Voltage\" "
   )

   ("leaf,l",
    po::value<std::string>(&params_.leaf)
    ->value_name("name"),
    "Set the resource leaf name. \n"
    "Example :\n"
    " --leaf \"__dp_read__\" "
   )

   ("pending",
    po::value<bool>()
    ->zero_tokens()
    ->default_value(false),
    "Set resource status pending")

   ("failed",
    po::value<bool>()
    ->zero_tokens()
    ->default_value(false),
    "Set resource status failed")

   ("missing",
    po::value<bool>()
    ->zero_tokens()
    ->default_value(false),
    "Set resource status missing")

   ("disabled",
    po::value<bool>()
    ->zero_tokens()
    ->default_value(false),
    "Set resource status disabled")

   ; // end of options' description

   return;
}

void app_params::print_usage(boost::program_options::options_description & opts_,
                             ostream                                     & out_)
{
   static const string APP_NAME = "status_event_producer";
   out_ << "\n" << APP_NAME << " -- " << endl;
   out_ << endl;
   out_ << "Usage : " << endl;
   out_ << std::endl;
   out_ << "  " << APP_NAME << " [OPTIONS] " << endl;
   out_ << opts_ << endl;
   return;
}


