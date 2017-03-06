#! /bin/bash

#run this script to generate a single dataset for the framework
#one should provide a name for the generated dataset
#we assume tcpdump is in the same directory as this script
#we also assume jsonparser.cpp is also in the same directory to run
#usage: ./ftpdumptest.sh <file_name>

sudo camflow --track-file tcpdump propagate

echo -e "Please enter the path of the file to run tcpdump: "

read file

tcpdump -r $file

sudo cp /tmp/audit.log tmp.txt

clang++ -std=c++0x -stdlib=libc++ -lc++ jsonparser.cpp -o jsonparser

./jsonparser tmp.txt $1

rm tmp.txt
