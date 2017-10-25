#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/event_listener --host          $RABBIT_HOST                                                      \
                                --port          $RABBIT_CLI                                                       \
                                --login         "vireserver"                                                      \
                                --password      "vireserver"                                                      \
                                --vhost         "/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp"  \
                                --topic_0       "#"



