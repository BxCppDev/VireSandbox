
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
#include <vire/cmslapp/connection_request.h>

// RabbitMQ
#include "rabbitmq/parameters.h"
#include "rabbitmq/connection.h"
#include "rabbitmq/channel.h"

using namespace std;


void print_usage (boost::program_options::options_description & opts_,
                  ostream                                     & out_)
{
   static const string APP_NAME = "connection_request";
   out_ << "\n" << APP_NAME << " -- " << endl;
   out_ << endl;
   out_ << "Usage : " << endl;
   out_ << std::endl;
   out_ << "  " << APP_NAME << " [OPTIONS] " << endl;
   out_ << opts_ << endl;
   return;
}

int main (int argc_, char* argv_ [])
{
   vire::initialize ();

   const string     snemo       = "SuperNEMO";
   const string     vhost       = "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp";
   const string     exchange    = "subcontractor.service";
   const string     login       = "vireserver";
   const string     passwd      = "vireserver";
   string           routing_key = "connection";
   string           host        = "localhost";
   uint16_t         port        = 5671;
   vector <string>  resources;


   namespace po = boost::program_options;
   po::options_description opts ("Main options");

   opts.add_options ()

       ("help,?",
        po::value <bool> ()->zero_tokens ()->default_value(false),
        "Produce help message")

       ("host,h",
        po::value <std::string> ()->value_name ("host"),
        "Set the rabbitmq server host. \n"
        "Example :\n"
        " --host \"caerabbitmq.in2p3.fr\"")

       ("port,p",
        po::value <uint16_t> ()->value_name ("port"),
        "Set the rabbitmq server port number. \n"
        "Example :\n"
        " --port 5671")

       ("resource,r",
        po::value <vector <string>> ()->value_name ("resource"),
        "Resource path list\n"
        "Example :\n"
        " -r /path/to/resource_1\n"
        " -r /path/to/resource_2\n"
        " -r  ...\n");

    po::variables_map vm;
    try {
        po::store (po::parse_command_line (argc_, argv_, opts), vm);
    } catch (po::error &e) {
        cerr << "ERROR: " << e.what () << endl << endl;
        print_usage (opts, cout);
        return EXIT_FAILURE;
    }
    if (vm.count ("help")) {
       if (vm ["help"].as <bool> ()) {
          print_usage (opts, cout);
          return EXIT_SUCCESS;
       }
    }
    if (vm.count ("host"))      host      = vm ["host"].as <string> ();
    if (vm.count ("port"))      port      = vm ["port"].as <uint16_t> ();
    if (vm.count ("resource"))  resources = vm ["resource"].as <vector <string>> ();
    if (resources.size () == 0) {
        print_usage (opts, cout);
        return EXIT_FAILURE;
    }

    // Message:
    vire::message::message req_msg;

    // Header:
    vire::message::message_header     & req_msg_header     = req_msg.grab_header ();
    vire::message::message_identifier   req_msg_id           ("vire.server", 42);
    vire::utility::model_identifier     req_body_layout_id;
    req_body_layout_id.set_name       (vire::message::body_layout::name());
    req_body_layout_id.set_version    (1);
    req_msg_header.set_body_layout_id (req_body_layout_id);
    req_msg_header.set_message_id     (req_msg_id);
    req_msg_header.set_timestamp      (vire::time::now ());
    req_msg_header.set_category       (vire::message::MESSAGE_REQUEST);

    // Payload:
    vire::utility::instance_identifier  snemo_id ("SuperNEMO_Demonstrator");
    vire::cmslapp::connection_request   con_req;
    con_req.set_setup_id (snemo_id);
    for (vector <string>::iterator itr = resources.begin (); itr != resources.end (); ++itr) {
    	con_req.add_requested_resource (snemo + ":" + *itr);
    }
    clog << endl;
    con_req.tree_dump (clog, "Connection request: ");
    clog << endl;

    // Body:
    vire::message::message_body & req_msg_body = req_msg.grab_body ();
    req_msg_body.set_payload (con_req);
   ///////////
    vire::utility::model_identifier payload_type_id1 = req_msg_body.get_payload_type_id ();
    vire::utility::model_identifier payload_type_id2;
    payload_type_id2.set (payload_type_id1.get_name (), 1);
    req_msg_body.set_payload_type_id (payload_type_id2);
   ///////////

    req_msg.tree_dump (std::clog, "Req. Message: ");
    clog << endl;

    // Generate protobufized message:
    ostringstream req_protobuf;
    protobuftools::store (req_protobuf, req_msg, 0);

    // Rabbit connection
    const bool                      pub_confirm  = true;
    rabbitmq::connection_parameters c_par;
                                    c_par.host   = host;
                                    c_par.port   = port;
                                    c_par.vhost  = vhost;
                                    c_par.login  = login;
                                    c_par.passwd = passwd;
    rabbitmq::connection            con (c_par, pub_confirm);
    rabbitmq::channel &             chan         = con.grab_channel ();

    string                          response;
    uint64_t                        delivery;
    rabbitmq::basic_properties      prop_out;
    rabbitmq::basic_properties      prop_in;
    rabbitmq::queue_parameters      q_par;
    rabbitmq::exchange_parameters   x_par;
    x_par.name      = exchange;
    x_par.type      = "topic";
    q_par.name      = "";
    q_par.exclusive = true;
    chan.queue_declare (q_par);
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

   vire::terminate();
   return EXIT_SUCCESS;
}




