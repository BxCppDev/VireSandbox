#!/bin/bash

echo ""
echo "   VHOSTS :"
echo ""

vhosts=$(sudo rabbitmqctl -q list_vhosts | awk '{print $1}')

for v in $vhosts; do
   echo
   echo "------------------------------------------"
   echo "    $v"
   echo "------------------------------------------"
   sudo rabbitmqctl list_exchanges -p $v
   sudo rabbitmqctl list_queues    -p $v
done

echo ""
