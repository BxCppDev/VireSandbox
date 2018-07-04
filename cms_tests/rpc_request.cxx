
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
#include <vire/cms/resource_exec.h>

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
   string   direct_queue_name;
   string   exchange_name;
   bool     exchange = false;

   //  request
   string   mode;
   string   setup;
   string   leaf;
   string   dirs;
   string   arg0;  //  "key=value"       //  todo more flexible nb of args
   string   arg1;
   string   arg2;
   string   arg3;
   bool     async = false;
   string   async_address;

   static void build_options (boost::program_options::options_description & opts_,
                              app_params                                  & params_);
   static void print_usage   (boost::program_options::options_description & opts_,
                              ostream                                     & out_);

};

struct key_val
{
   string key;
   string val;
};

void key_val_parse (const string & arg_str, struct key_val & kv)
{
   //  string arg_str = "key=val"
   auto eq_pos = arg_str.find ("=");
   kv.key      = arg_str.substr (0, eq_pos);
   kv.val      = arg_str.substr (eq_pos+1);
}

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
   struct key_val kv;

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
      if (params.direct_queue_name.empty ()) {
         params.exchange = true;
      }
      if (params.exchange_name.empty ()) {
         params.exchange_name = "resource_request";
      }
      if (params.async_address.empty ()) {
         params.async = false;
         params.mode  = "sync_resource_exec_request";
      } else {
         params.async = true;
         params.mode = "async_resource_exec_request";
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
      if (params.arg0.empty()) {
         nb_req_args = 0;
      } else if (params.arg1.empty()) {
         nb_req_args = 1;
      } else if (params.arg2.empty()) {
         nb_req_args = 2;
      } else if (params.arg3.empty()) {
         nb_req_args = 3;
      } else {
         nb_req_args = 4;
      }
//         if (params.mode == "async_resource_exec_request") {
//            // Async return queue:
//            params.async_address = "snemo.vire.client_0.Die0baxu.q";
//         }

      string resource_path = vire::utility::path::build      (params.setup, params.dirs, params.leaf);
      string routing_key   = vire::utility::path::to_address (resource_path);

      clog <<    endl;
      clog <<    "--------------------------------------------------------------------------------------" << endl;

      clog <<    "[info] host              " << params.host   << endl;
      clog <<    "[info] port              " << params.port   << endl;
      clog <<    "[info] vhost             " << params.vhost  << endl;
      clog <<    "[info] login             " << params.login  << endl;
      if (params.exchange) {
         clog << "[info] req.exchange      " << params.exchange_name      << endl;
      } else {
         clog << "[info] req.direct_queue  " << params.direct_queue_name  << endl;
      }
      clog <<    "[info] resource path     " << resource_path << endl;
      clog <<    "[info] routing key       " << routing_key << endl;
      if (params.async) {
         clog << "[info] async address     " << params.async_address << endl;
      }

      // Message:
      vire::message::message req_msg;

      // Header:
      vire::message::message_header     & req_msg_header     = req_msg.grab_header ();
      vire::message::message_identifier   req_msg_id           ("vire.server", 42);
      vire::utility::model_identifier     req_body_layout_id;
//         std::clog << "[info] Vire req message body layout name    : " << vire::message::body_layout::name () << std::endl;
//         std::clog << "[info] Vire req message body layout version : " << vire::message::body_layout::current_version () << std::endl;
///////////
      req_body_layout_id.set_name       (vire::message::body_layout::name());
      req_body_layout_id.set_version    (1);
      //req_body_layout_id.set_version    (vire::message::body_layout::current_version());
///////////
      //
      req_msg_header.set_body_layout_id (req_body_layout_id);
      req_msg_header.set_message_id     (req_msg_id);
      req_msg_header.set_timestamp      (vire::time::now ());
////
      req_msg_header.set_category       (vire::message::MESSAGE_REQUEST);
////
      //req_msg_header.set_asynchronous   (!params.async_address.empty ());
      req_msg_header.set_asynchronous   (params.async);
      req_msg_header.set_async_address  (params.async_address);
      req_msg_header.add_metadata       ("user_correlation_id", routing_key);

      // Payload:
      auto rer_ptr = std::make_shared<vire::cms::resource_exec>();
      auto & rer = *rer_ptr;
      rer.set_path (resource_path);
      if (nb_req_args > 0) {
         clog << "[info] req_arg0          " << params.arg0 << endl;
         key_val_parse (params.arg0, kv);
         rer.add_input_argument (kv.key, kv.val);
      }
      if (nb_req_args > 1) {
         clog << "[info] req_arg1          " << params.arg1 << endl;
         key_val_parse (params.arg1, kv);
         rer.add_input_argument (kv.key, kv.val);
      }
      if (nb_req_args > 2) {
         key_val_parse (params.arg2, kv);
         rer.add_input_argument (kv.key, kv.val);
         clog << "[info] req_arg2          " << params.arg2 << endl;
      }
      if (nb_req_args > 3) {
         clog << "[info] req_arg3          " << params.arg3 << endl;
         key_val_parse (params.arg3, kv);
         rer.add_input_argument (kv.key, kv.val);
      }
      std::clog << std::endl;
      rer.tree_dump (std::clog, "Resource execution request: ");
      std::clog << std::endl;

      // Body:
      vire::message::message_body & req_msg_body = req_msg.grab_body ();
      req_msg_body.set_payload (rer_ptr);
///////////
      vire::utility::model_identifier payload_type_id1 = req_msg_body.get_payload_type_id ();
      vire::utility::model_identifier payload_type_id2;
      payload_type_id2.set (payload_type_id1.get_name (), 1);
      req_msg_body.set_payload_type_id (payload_type_id2);
///////////

      req_msg.tree_dump(std::clog, "Req. Message: ");
      std::clog << std::endl;

      // Generate protobufized message:
      std::ostringstream req_protobuf;
      protobuftools::store (req_protobuf, req_msg, 0);

      // Rabbit connection
      const bool                      pub_confirm  = true;
      rabbitmq::connection_parameters c_par;
                                      c_par.host   = params.host;
                                      c_par.port   = params.port;
                                      c_par.vhost  = params.vhost;
                                      c_par.login  = params.login;
                                      c_par.passwd = params.passwd;
      rabbitmq::connection            con (c_par, pub_confirm);
      rabbitmq::channel &             chan = con.grab_channel ();

      string                          response;
      uint64_t                        delivery;
      rabbitmq::basic_properties      prop_out;
      rabbitmq::basic_properties      prop_in;
      rabbitmq::queue_parameters      q_par;
      rabbitmq::exchange_parameters   x_par;
      if (params.exchange) {
         x_par.name      = params.exchange_name;
         x_par.type      = "topic";
//            chan.exchange_declare (x_par);
      }
      q_par.name      = "";
      q_par.exclusive = true;
      chan.queue_declare          (q_par);
      clog << "rab queue " << q_par.name << endl;
      chan.basic_consume          (q_par.name, "", true);
      prop_out.set_correlation_id ("corid_" + std::to_string (std::rand ()));
      prop_out.set_reply_to       (q_par.name);
      if (params.exchange) {
         chan.basic_publish (x_par.name, routing_key, req_protobuf.str (), prop_out);
      } else {
         chan.basic_publish ("", params.direct_queue_name, req_protobuf.str (), prop_out);
      }
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

   ("direct_queue_name",
    po::value<std::string>(&params_.direct_queue_name)
    ->value_name("name"),
    "Set request directly passing on a queue \n"
    "Example :\n"
    " --direct_queue_name PrivateQueue"
   )

   ("exchange_name",
    po::value<std::string>(&params_.exchange_name)
    ->value_name("name"),
    "Set request passing by exchange. \n"
    "Example :\n"
    " --exchange_name PublicExchangeName"
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

   ("arg0",
    po::value<std::string>(&params_.arg0)
    ->value_name("arg0"),
    "Append \"a_key=a_value\" as request argument #0. \n"
    "Example :\n"
    " --arg0 \"refresh=true\" "
   )

   ("arg1",
    po::value<std::string>(&params_.arg1)
    ->value_name("arg1"),
    "Append \"a_key=a_value\" as request argument #1. \n"
   )

   ("arg2",
    po::value<std::string>(&params_.arg2)
    ->value_name("arg1"),
    "Append \"a_key=a_value\" as request argument #2. \n"
   )

   ("arg3",
    po::value<std::string>(&params_.arg3)
    ->value_name("arg3"),
    "Append \"a_key=a_value\" as request argument #3. \n"
   )
   ; // end of options' description

   return;
}

void app_params::print_usage(boost::program_options::options_description & opts_,
                             ostream                                     & out_)
{
   static const string APP_NAME = "rpc_request";
   out_ << "\n" << APP_NAME << " -- " << endl;
   out_ << endl;
   out_ << "Usage : " << endl;
   out_ << std::endl;
   out_ << "  " << APP_NAME << " [OPTIONS] " << endl;
   out_ << opts_ << endl;
   return;
}


