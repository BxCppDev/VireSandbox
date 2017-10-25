#!/bin/bash

#if [ $# -lt 2 ]; then
#   echo "usage   :"
#   echo "          $0  exchange_name  topic"
#   echo "example :"
#   echo "          $0  log.event  SuperNEMO.Demonstrator.CMS.Coil.PS_I0.#"
#   exit
#fi
#exchange_name=$1
#topic=$2

. SET_RABBIT_HOST_PORT.sh

./_install.d/bin/event_listener  --host              $RABBIT_HOST                                   \
                                 --port              $RABBIT_CLI                                    \
                                 --login             "vireserver"                                   \
                                 --password          "vireserver"                                   \
                                 --vhost             "/supernemo/demonstrator/cms/vire/monitoring"



