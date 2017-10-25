#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/event_listener --host          $RABBIT_HOST                                              \
                                --port          $RABBIT_CLI                                               \
                                --login         "wxCVbn"                                                  \
                                --password      "wxCVbn"                                                  \
                                --vhost         "/supernemo/demonstrator/cms/vire/clients/system/wxCVbn"  \
                                --exchange_name "vireserver.event"                                        \
                                --topic_0       "#"


