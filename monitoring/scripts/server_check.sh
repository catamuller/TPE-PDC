#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <IP_address> <port>"
    exit 1
fi

IP_ADDRESS=$1
PORT=$2

# Attempt to connect to the SMTP server
if nc -z "$IP_ADDRESS" "$PORT"; then
  echo "true"
else
  echo "false"
fi