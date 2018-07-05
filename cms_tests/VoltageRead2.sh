#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/rpc_request  --host              $RABBIT_HOST                                      \
                              --port              $RABBIT_CLI                                       \
                              --login             "wxCVbn"                                          \
                              --password          "wxCVbn"                                          \
                              --vhost             "/supernemo/demonstrator/cms/vire/monitoring"     \
			      --direct_queue_name "amq.gen-eO0EA2LipjoAl7PB3hU_Kw"                  \
                              --dirs              "/Demonstrator/CMS/Coil/PS_I0/Monitoring/Voltage" \
                              --leaf              "__dp_read__"



###                              --exchange_name     "resource_request.service"                        
