#!/bin/bash

   USER="wxCVbn"
   VH_CTRL="/supernemo/demonstrator/cms/vire/control"
   VH_MONI="/supernemo/demonstrator/cms/vire/monitoring"
   VH_CLIE="/supernemo/demonstrator/cms/vire/clients/system/$USER"
   
   #  exchanges & regexps ...
   AMQGEN="^amq\\.gen*"
   AMQDEF="^amq\\.default$"
   X_LOG="^log\\.event$"
   X_ALARM="^alarm\\.event$"
   X_PSUB="^pubsub\\.event$"
   X_VIR_S="^vireserver\\.service$"
   X_VIR_E="^vireserver\\.event$"
   #sudo rabbitmqctl exchange_declare $VH_CLIE   "vireserver.service"        topic true false false
   #sudo rabbitmqctl exchange_declare $VH_CLIE  "vireserver.event"          topic true false false

   sudo rabbitmqctl add_vhost          $VH_CLIE
   sudo rabbitmqctl add_user           $USER    $USER
   sudo rabbitmqctl set_permissions -p $VH_CTRL $USER "$AMQGEN" "$X_QUEST"                  "$AMQGEN"
   sudo rabbitmqctl set_permissions -p $VH_MONI $USER "$AMQGEN" "$AMQDEF|$AMQGEN|$X_QUEST"  "$AMQGEN|$X_LOG|$X_ALARM|$X_PSUB" 
   sudo rabbitmqctl set_permissions -p $VH_CLIE $USER "$AMQGEN" "$AMQDEF"                   "$AMQGEN|$X_VIR_E|$X_VIR_S"

