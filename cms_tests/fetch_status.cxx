
// Standard library:
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <memory>

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
#include <vire/cms/resource_fetch_status.h>

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

   //  request
   string   mode;
   string   setup;
   string   leaf;
   string   dirs;
   bool     async = false;
   string   async_address;

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
   int            nb_req_args = 0;

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
      if (params.async_address.empty ()) {
         params.async = false;
         params.mode  = "sync_fetch_status_request";
      } else {
         params.async = true;
         params.mode = "async_fetch_status_request";
      }
      if (params.setup.empty()) {
         params.setup = "SuperNEMO";
      }
      if (params.dirs.empty()) {
         params.dirs = "/Demonstrator/CMS/Coil/PS_I0/Monitoring/Voltage";
         //params.dirs = "/Demonstrator/CMS/Coil/PS_I0/Control/Voltage";
         //params.dirs = "/Demonstrator/CMS/Coil/PS_I0"; //  vrai alim  (executon async dans le decor)
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
      clog <<    "[info] resource path     " << resource_path << endl;
      clog <<    "[info] routing key       " << routing_key << endl;
      if (params.async) {
         clog << "[info] async address     " << params.async_address << endl;
      }

      // Message:
      vire::message::message fs_msg;

      // Header:
      vire::message::message_header     & fs_msg_header     = fs_msg.grab_header ();
      vire::message::message_identifier   fs_msg_id           ("vire.server", 42);
      vire::utility::model_identifier     fs_body_layout_id;
//         std::clog << "[info] Vire req message body layout name    : " << vire::message::body_layout::name () << std::endl;
//         std::clog << "[info] Vire req message body layout version : " << vire::message::body_layout::current_version () << std::endl;
///////////
      fs_body_layout_id.set_name       (vire::message::body_layout::name());
      fs_body_layout_id.set_version    (1);
      //req_body_layout_id.set_version    (vire::message::body_layout::current_version());
///////////
      //
      fs_msg_header.set_body_layout_id (fs_body_layout_id);
      fs_msg_header.set_message_id     (fs_msg_id);
      fs_msg_header.set_timestamp      (vire::time::now ());
////
      fs_msg_header.set_category       (vire::message::MESSAGE_REQUEST);
////
      //fs_msg_header.set_asynchronous   (!params.async_address.empty ());
      fs_msg_header.set_asynchronous   (params.async);
      fs_msg_header.set_async_address  (params.async_address);
      fs_msg_header.add_metadata       ("user_correlation_id", routing_key);

      // Payload:
      auto fsr_ptr = std::make_shared<vire::cms::resource_fetch_status>();

      auto & fsr = *fsr_ptr;
      fsr.set_path (resource_path);
      std::clog << std::endl;
      fsr.tree_dump (std::clog, "Fetch status request: ");
      std::clog << std::endl;

      // Body:
      vire::message::message_body & fs_msg_body = fs_msg.grab_body ();
      fs_msg_body.set_payload (fsr_ptr);
///////////
      vire::utility::model_identifier payload_type_id1 = fs_msg_body.get_payload_type_id ();
      vire::utility::model_identifier payload_type_id2;
      payload_type_id2.set (payload_type_id1.get_name (), 1);
      fs_msg_body.set_payload_type_id (payload_type_id2);
///////////

      fs_msg.tree_dump(std::clog, "Fetch status message: ");
      std::clog << std::endl;

      // Generate protobufized message:
      std::ostringstream req_protobuf;
      protobuftools::store (req_protobuf, fs_msg, 0);

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

      string                          response;
      uint64_t                        delivery;
      rabbitmq::basic_properties      prop_out;
      rabbitmq::basic_properties      prop_in;
      rabbitmq::queue_parameters      q_par;
      rabbitmq::exchange_parameters   x_par;
      x_par.name      = "resource_request.service";
      x_par.type      = "topic";
      q_par.name      = "";
      q_par.exclusive = true;
      chan.queue_declare          (q_par);
      clog << "rab queue " << q_par.name << endl;
      chan.basic_consume          (q_par.name, "", true);
      prop_out.set_correlation_id ("corid_" + std::to_string (std::rand ()));
      prop_out.set_reply_to       (q_par.name);
      chan.basic_publish (x_par.name, routing_key, req_protobuf.str (), prop_out);
      while (1) {
         chan.consume_message (response, routing_key, prop_in, delivery);
         if (not prop_in.has_correlation_id ())                               continue;
         if (prop_in.get_correlation_id () == prop_out.get_correlation_id ()) break;
      }
      //  clog << " [x] Got " << response << endl;
      stringstream resp_protobuf;
      resp_protobuf << response;
      vire::message::message resp_msg;
      protobuftools::load (resp_protobuf, resp_msg, 0);

      // Dump message:
      resp_msg.tree_dump (std::clog, "Response : ");
      clog <<    "--------------------------------------------------------------------------------------" << endl;
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

   ("async_address,a",
    po::value<std::string>(&params_.async_address)
    ->value_name ("async_address"),
    "Set the asynchronous address. \n"
    "Example :\n"
    " --async_address amq.gen.AzErTy0YtReZa "
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

   ; // end of options' description

   return;
}

void app_params::print_usage(boost::program_options::options_description & opts_,
                             ostream                                     & out_)
{
   static const string APP_NAME = "fetch_status";
   out_ << "\n" << APP_NAME << " -- " << endl;
   out_ << endl;
   out_ << "Usage : " << endl;
   out_ << std::endl;
   out_ << "  " << APP_NAME << " [OPTIONS] " << endl;
   out_ << opts_ << endl;
   return;
}


