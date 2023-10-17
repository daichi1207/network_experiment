#!/bin/bash

if [ "$(docker ps | grep "node" | wc -l)" -ne 5 ]; then
    echo "make sure all containers are running"
    exit 1
fi

NODES=("node1" "node2" "node3" "node4" "node5")

LOGFILE="server_bandwidth.log"

# Clear previous log file
> $LOGFILE

for SERVER in "${NODES[@]}"; do
    # Start iperf3 server on the server node
    docker exec -d $SERVER iperf3 -s --json &
    SERVER_PID=$!

    for CLIENT in "${NODES[@]}"; do
        if [ "$SERVER" != "$CLIENT" ]; then
            # Initiate iperf3 client connection to server
            docker exec $CLIENT iperf3 -c $SERVER -b 60M -t 10
        fi
    done
    
    SERVER_LOG=$(docker exec $SERVER cat /tmp/iperf3-logs)
    echo "Server-side bandwidth measurements for $SERVER:" >> $LOGFILE
    echo "$SERVER_LOG" | jq '.end.sum_received.bits_per_second' >> $LOGFILE
    
    docker kill $SERVER_PID
done

cat $LOGFILE

