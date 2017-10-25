#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/rpc_request --host          $RABBIT_HOST                                                     \
                             --port          $RABBIT_CLI                                                      \
                             --login         "cmslapp"                                                        \
                             --password      "cmslapp"                                                        \
                             --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp" \
                             --exchange_name "vireserver.service"                                             \
                              --dirs         "/Demonstrator/CMS/Coil/PS_I0/Control/Voltage"                   \
                              --leaf         "__dp_write__"                                                   \
                              --arg0         "setting=5.0"



