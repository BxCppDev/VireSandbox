#!/bin/bash

   #  vhosts
   VH_GATE="/supernemo/demonstrator/cms/vire/clients/gate"
   VH_CTRL="/supernemo/demonstrator/cms/vire/control"
   VH_MONI="/supernemo/demonstrator/cms/vire/monitoring"
   VH_CMS="/supernemo/demonstrator/cms/vire/subcontractors/system/cmslapp"
   
   sudo rabbitmqctl add_vhost $VH_GATE
   sudo rabbitmqctl add_vhost $VH_CTRL
   sudo rabbitmqctl add_vhost $VH_MONI
   sudo rabbitmqctl add_vhost $VH_CMS

   #  exchanges & regexps ...
   AMQGEN="^amq\\.gen*"
   AMQDEF="^amq\\.default$"
   X_GATE="^gate\\.service$"
   X_QUEST="^resource_request\\.service$"
   X_LOG="^log\\.event$"
   X_ALARM="^alarm\\.event$"
   X_PSUB="^pubsub\\.event$"
   X_SUB_S="^subcontractor\\.service$"
   X_SUB_E="^subcontractor\\.event$"
   X_VIR_S="^vireserver\\.service$"
   X_VIR_E="^vireserver\\.event$"
   #sudo rabbitmqctl exchange_declare $VH_GATE  "gate.service"              topic true false false
   #sudo rabbitmqctl exchange_declare $VH_CTRL  "resource_request.service"  topic true false false
   #sudo rabbitmqctl exchange_declare $VH_MONI  "resource_request.service"  topic true false false
   #sudo rabbitmqctl exchange_declare $VH_MONI  "log.event"                 topic true false false
   #sudo rabbitmqctl exchange_declare $VH_MONI  "alarm.event"               topic true false false
   #sudo rabbitmqctl exchange_declare $VH_MONI  "pubsub.event"              topic true false false
   #sudo rabbitmqctl exchange_declare $VH_CMS   "vireserver.service"        topic true false false
   #sudo rabbitmqctl exchange_declare $VH_CMS   "vireserver.event"          topic true false false
   #sudo rabbitmqctl exchange_declare $VH_CMS   "subcontractor.service"     topic true false false
   #sudo rabbitmqctl exchange_declare $VH_CMS   "subcontractor.event"       topic true false false

   #  users
   USER="supernemo_adm"
   sudo rabbitmqctl add_user        $USER $USER
   sudo rabbitmqctl set_user_tags   $USER administrator management
   sudo rabbitmqctl set_permissions -p "/"      $USER ".*" ".*" ".*"
   sudo rabbitmqctl set_permissions -p $VH_GATE $USER ".*" ".*" ".*"
   sudo rabbitmqctl set_permissions -p $VH_CTRL $USER ".*" ".*" ".*"
   sudo rabbitmqctl set_permissions -p $VH_MONI $USER ".*" ".*" ".*"
   sudo rabbitmqctl set_permissions -p $VH_CMS  $USER ".*" ".*" ".*"
   
   USER="vireserver"
   sudo rabbitmqctl add_user           $USER    $USER 
   sudo rabbitmqctl set_permissions -p $VH_GATE $USER "$AMQGEN" "$AMQDEF|$AMQGEN"                                     "$X_GATE|$AMQGEN"
   sudo rabbitmqctl set_permissions -p $VH_CTRL $USER "$AMQGEN" "$AMQDEF|$AMQGEN"                                     "$X_QUEST|$AMQGEN"
   sudo rabbitmqctl set_permissions -p $VH_MONI $USER "$AMQGEN" "$AMQDEF|$AMQGEN|$X_QUEST|$X_LOG|$X_ALARM|$X_PSUB"    "$X_QUEST|$AMQGEN|$X_LOG|$X_ALARM|$X_PSUB" 
   sudo rabbitmqctl set_permissions -p $VH_CMS  $USER "$AMQGEN" "$AMQDEF|$AMQGEN|$X_SUB_S|$X_VIR_E|$X_VIR_S|$X_SUB_E" "$X_VIR_S|$AMQGEN|$X_SUB_E|$X_SUB_S|$X_VIR_E"

   USER="cmslapp"
   sudo rabbitmqctl add_user           $USER $USER
   sudo rabbitmqctl set_permissions -p $VH_MONI $USER "$AMQGEN" "$AMQDEF|$AMQGEN|$X_LOG|$X_ALARM|$X_PSUB"             "$X_QUEST|$AMQGEN"
   sudo rabbitmqctl set_permissions -p $VH_CMS  $USER "$AMQGEN" "$AMQDEF|$AMQGEN|$X_VIR_S|$X_SUB_E|$X_SUB_S|$X_VIR_E" "$X_SUB_S|$AMQGEN|$X_VIR_E|$X_VIR_S|$X_SUB_E" 

