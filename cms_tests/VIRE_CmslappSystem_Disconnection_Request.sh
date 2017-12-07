#!/bin/bash

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/disconnection_request --host   $RABBIT_HOST \
                                       --port   $RABBIT_CLI
