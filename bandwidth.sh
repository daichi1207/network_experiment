#!/bin/bash

if [ "$(docker ps | grep "node" | wc -l)" -ne 5 ]; then
    echo "make sure all containers are running"
    exit 1
fi

NODES=("node1" "node2" "node3" "node4" "node5")

for NODE in "${NODES[@]}"; do
    docker exec $NODE pkill iperf3
    echo "Killed iperf3 processes on $NODE"
done

LOGFILE="server_bandwidth.log"

> $LOGFILE

for SERVER in "${NODES[@]}"; do
    docker exec -d $SERVER sh -c "iperf3 -s --json > /tmp/iperf3-logs 2>&1 &"

    for CLIENT in "${NODES[@]}"; do
        if [ "$SERVER" != "$CLIENT" ]; then
            docker exec $CLIENT iperf3 -c $SERVER -b 60M -t 4
            sleep 2   
        fi
    done
    
    SERVER_LOG=$(docker exec $SERVER cat /tmp/iperf3-logs 2>/dev/null)
    
    if [ ! -z "$SERVER_LOG" ]; then
        echo "Server-side bandwidth measurements for $SERVER:" >> $LOGFILE
        echo "$SERVER_LOG" | jq '.end.sum_received.bits_per_second' >> $LOGFILE
    else
        echo "Failed to retrieve logs for $SERVER." >> $LOGFILE
    fi

    # Kill iperf3 server process
    docker exec $SERVER pkill iperf3
    sleep 2
done

