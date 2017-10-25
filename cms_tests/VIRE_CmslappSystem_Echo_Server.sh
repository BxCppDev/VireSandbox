#!/bin/bash


. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/echo_exchange_server --host          $RABBIT_HOST                                                      \
                                      --port          $RABBIT_CLI                                                       \
                                      --login         "vireserver"                                                      \
                                      --password      "vireserver"                                                      \
                                      --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp"  \
                                      --exchange_name "vireserver.service"                                              \
                                      --topic_0       "#"



