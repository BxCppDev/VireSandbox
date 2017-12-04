#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/rpc_request --host          $RABBIT_HOST                                                     \
                             --port          $RABBIT_CLI                                                      \
                             --login         "vireserver"                                                     \
                             --password      "vireserver"                                                     \
                             --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp" \
                             --exchange_name "subcontractor.service"                                          \
                              --dirs         "/Demonstrator/CMS/Coil/PS_I0/Monitoring/Voltage"                \
                              --leaf         "__dp_subscribe__"                                               \
                              --arg0         "subscription=false"



