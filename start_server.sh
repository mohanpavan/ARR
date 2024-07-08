#!/bin/bash

SERVER_MAC=$(ifconfig docker0 | grep ether | awk '{print $2}')
echo "./server 1024 $SERVER_MAC"
./server 1024 $SERVER_MAC