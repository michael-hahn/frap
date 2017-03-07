#!/bin/bash

#This script is to run on the client side of the wget exploit.
#We assume that wget is of version prior to 1.18; otherwise, no exploit will take place.
#we also assume that the script is in the superuser mode, and run in /root directory
#One must input the IP address of the server to retrieve data
#One must also provide a path to save the provenance file
#We assume jsonparser is in the same working directory to run

#usage: ./wgettest_client.sh <IP_address> <destination_file_path>

sudo camflow --track-file /usr/local/bin/wget propagate

wget -N $1

sleep 2s

wget -N $1

sleep 2s

wget -N $1

sleep 2s

wget -N $1

sleep 2s

wget -N $1

sleep 2s

wget -N $1

sleep 4s

clang++ -std=c++0x -stdlib=libc++ -lc++ jsonparser.cpp -o jsonparser

./jsonparser /tmp/audit.log $1