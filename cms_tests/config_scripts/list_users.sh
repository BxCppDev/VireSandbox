#!/bin/bash

echo ""
echo "   USERS : "
echo ""

users=$(sudo rabbitmqctl -q list_users | awk '{print $1}')

for u in $users; do
   echo
   echo "------------------------------------------"
   echo "    $u"
   echo "------------------------------------------"
   sudo rabbitmqctl list_user_permissions $u
done

echo ""
