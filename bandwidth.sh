# #!/bin/sh
#
#
# if [ "$(docker ps | grep "node" | wc -l)" -ne 5 ]; then
#   echo "make sure all containers are running"
#   exit 1
# fi
#
# docker exec -d node1 iperf3 -s
#
# # Give the server a moment to start
# sleep 2
#
# BANDWIDTH=$(docker exec node3 iperf3 -u -c node1 -t 10 -b 30M | grep "receiver" | awk '{print $7, $8}')
#
# docker exec node1 pkill iperf3
#
# echo "Bandwidth from node3 to node1: $BANDWIDTH"
#

if [ "$(docker ps | grep "node" | wc -l)" -ne 5 ]; then
  echo "make sure all containers are running"
  exit 1
fi

NODES=("node1" "node2" "node3" "node4" "node5")

for SERVER in "${NODES[@]}"; do
    # Start iperf3 server on the current node
    docker exec -d $SERVER iperf3 -s

    sleep 2
    for CLIENT in "${NODES[@]}"; do
        sleep 1
        if [ "$SERVER" != "$CLIENT" ]; then
            echo "starting iperf3 client on $CLIENT"
            docker exec $CLIENT iperf3 -c $SERVER -t 3 -b 60M > /dev/null

            # Capture server-side bandwidth
            SERVER_OUTPUT=$(docker exec $SERVER iperf3 -s --json)
            BANDWIDTH=$(echo $SERVER_OUTPUT | jq '.end.sum_received.bits_per_second')
            echo "Server-side bandwidth from $CLIENT to $SERVER: $BANDWIDTH bps"

            # Kill the iperf3 server process
            docker exec $SERVER pkill iperf3
            sleep 1
            # Restart the iperf3 server for the next client
            docker exec -d $SERVER iperf3 -s
        fi
    done
    docker exec $SERVER pkill iperf3
done

