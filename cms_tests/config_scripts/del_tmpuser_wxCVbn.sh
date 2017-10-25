#!/bin/bash

   USER="wxCVbn"
   VH_CLIE="/supernemo/demonstrator/cms/vire/clients/system/$USER"
   
   sudo delete_user  $USER
   sudo delete_vhost $VH_CLIE


