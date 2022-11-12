#!/bin/bash
if $(cat /etc/hosts | grep $(hostname)); then
    # hostname already added
    echo "hostname added"
else
    echo "127.0.0.1 $(hostname)" >> /etc/hosts
    echo "hostname added"
fi

/root/unitree_legged_sdk/build/zmq_walk
