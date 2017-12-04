#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/status_event_producer    --host           $RABBIT_HOST                                      \
                                          --port           $RABBIT_CLI                                       \
                                          --login          "vireserver"                                      \
                                          --password       "vireserver"                                      \
                                          --vhost          "/supernemo/demonstrator/cms/vire/monitoring"     \
                                          --exchange_name  "pubsub.event"                                    \
                                          --dirs           "/Demonstrator/CMS/Coil/PS_I0/Monitoring/Voltage" \
                                          --leaf           "__dp_read__"                                     \
                                          --pending 
#                                         --missing 
#                                         --disabled 
#                                         --failed



