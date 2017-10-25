#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/echo_exchange_server --host          $RABBIT_HOST                                   \
                                      --port          $RABBIT_CLI                                    \
                                      --login         "cmslapp"                                      \
                                      --password      "cmslapp"                                      \
                                      --vhost         "/supernemo/demonstrator/cms/vire/monitoring"  \
                                      --exchange_name "resource_request.service"                     \
                                      --topic_0       "#"



