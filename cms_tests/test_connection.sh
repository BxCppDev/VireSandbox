#!/bin/bash



#./_build.d/event_listener -h 172.17.0.10 -p 5671 -v /test -l supernemo_adm -w supernemo_adm

#./_build.d/event_listener -h 172.17.0.10 -p 5672 -v /test -l supernemo_adm -w supernemo_adm

openssl s_client -cipher AES256-SHA256 -connect 172.17.0.1:5671 | less
