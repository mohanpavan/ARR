#!/bin/bash

# Print available network interfaces
echo "Available network interfaces:"
ifconfig -a

# Fetch MAC addresses
SRC_MAC=$(ifconfig eth0 | grep ether | awk '{print $2}')
DEST_MAC=$(ifconfig docker0 | grep ether | awk '{print $2}')

# Print fetched MAC addresses
echo "SRC_MAC: $SRC_MAC"
echo "DEST_MAC: $DEST_MAC"

# Run the client with the fetched MAC addresses
echo "./client $SRC_MAC $DEST_MAC"
./client $SRC_MAC $DEST_MAC