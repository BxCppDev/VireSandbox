#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/rpc_request --host          $RABBIT_HOST                                                     \
                             --port          $RABBIT_CLI                                                      \
                             --login         "vireserver"                                                     \
                             --password      "vireserver"                                                     \
                             --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp" \
                             --exchange_name "subcontractor.service"                                          \
                             --dirs          "/Demonstrator/CMS/DAQ/FSM"                                      \
                             --leaf          "Stop"



