#!/bin/bash

if [ $# -eq 0 ]; then
   echo "usage : "
   echo "        $0  async_address"
   exit
fi
async_address=$1

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/rpc_request --host          $RABBIT_HOST                                                     \
                             --port          $RABBIT_CLI                                                      \
                             --login         "vireserver"                                                     \
                             --password      "vireserver"                                                     \
                             --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp" \
                             --exchange_name "subcontractor.service"                                          \
                             --dirs          "/Demonstrator/CMS/Coil/PS_I0"                                   \
                             --leaf          "HysteresisCycleStart"                                           \
                             --arg0          "nbCycle=2"                                                      \
                             --arg1          "rampDelay=2"                                                    \
                             --arg2          "stepDelay=1"                                                    \
                             --arg3          "Voltage=2"                                                      \
                             --async_address $async_address


