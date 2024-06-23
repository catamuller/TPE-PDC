#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <IP_address> <port>"
    exit 1
fi

IP_ADDRESS=$1
PORT=$2

SEPARATOR='\n'

declare -A connections

while true; do
    netstat -an | grep "$IP_ADDRESS:$PORT" | grep 'ESTABLISHED' | awk '{print $5}' | while read -r line; do
        if [[ "$line" != "$IP_ADDRESS:$PORT" && ! ${connections["$line"]} ]]; then
            echo "$line"
            connections["$line"]=1
        fi
    done
    printf "END_BATCH\n"
    sleep 1
    connections=()
done