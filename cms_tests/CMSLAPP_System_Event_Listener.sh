#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/event_listener --host          $RABBIT_HOST                                                      \
                                --port          $RABBIT_CLI                                                       \
                                --login         "cmslapp"                                                         \
                                --password      "cmslapp"                                                         \
                                --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp"  \
                                --exchange_name "vireserver.event"                                                \
                                --topic_0       "#"



