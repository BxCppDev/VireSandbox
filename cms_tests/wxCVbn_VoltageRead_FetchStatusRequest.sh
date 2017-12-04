#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/fetch_status --host              $RABBIT_HOST                                      \
                              --port              $RABBIT_CLI                                       \
                              --login             "wxCVbn"                                          \
                              --password          "wxCVbn"                                          \
                              --vhost             "/supernemo/demonstrator/cms/vire/monitoring"     \
                              --dirs              "/Demonstrator/CMS/Coil/PS_I0/Monitoring/Voltage" \
                              --leaf              "__dp_read__"



