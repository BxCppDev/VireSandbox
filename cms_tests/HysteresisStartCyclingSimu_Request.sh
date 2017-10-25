#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/rpc_request --host          $RABBIT_HOST                                                     \
                             --port          $RABBIT_CLI                                                      \
                             --login         "vireserver"                                                     \
                             --password      "vireserver"                                                     \
                             --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp" \
                             --exchange_name "subcontractor.service"                                          \
                             --dirs          "/Demonstrator/CMS/Coil/PS_I1"                                   \
                             --leaf          "HysteresisCycleStart"                                           \
                             --arg0          "nbCycle=1"                                                      \
                             --arg1          "rampDelay=1"                                                    \
                             --arg2          "stepDelay=1"                                                    \
                             --arg3          "Voltage=2"


