#!/bin/bash

#This script is to run on the client side of the wget exploit.
#We assume that wget is of version prior to 1.18; otherwise, no exploit will take place.
#we also assume that the script is in the superuser mode
#One must input the IP address of the server to retrieve data

#usage: ./wggettest_client.sh <IP_address>

cd /root

sudo camflow --track-file /usr/local/bin/wget propagate

wget -N $1

wget -N $1

wget -N $1

wget -N $1

wget -N $1

wget -N $1

sudo cp /tmp/audio.log provenance.txt