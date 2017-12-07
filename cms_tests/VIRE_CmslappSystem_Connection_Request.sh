#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/connection_request --host   $RABBIT_HOST                                                      \
                                    --port   $RABBIT_CLI                                                       \
                                    -r       /Demonstrator/CMS/DAQ/FSM                                         \
                                    -r       /Demonstrator/CMS/Coil/PS_I0/Control/Voltage/__dp_write__         \
                                    -r       /Demonstrator/CMS/Coil/PS_I0/Monitoring/Voltage/__dp_read__
