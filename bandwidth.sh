#!/bin/bash

 if [ "$(docker ps | grep "node" | wc -l)" -ne 5 ]; then
   echo "make sure all containers are running"
   exit 1
 fi

 NODES=("node1" "node2" "node3" "node4" "node5")

 echo "starting"
 for SERVER in "${NODES[@]}"; do
     # Start iperf3 server on the current node
     SERVER_OUTPUT=$(docker exec $SERVER iperf3 -s --json)

     for CLIENT in "${NODES[@]}"; do
         if [ "$SERVER" != "$CLIENT" ]; then
             echo "starting iperf3 client on $CLIENT"
             docker exec $CLIENT iperf3 -c $SERVER -t 3 -b 60M > /dev/null

 	    echo "done iperf3"
 	    sleep 3
 	    echo $SERVER_OUTPUT
             # BANDWIDTH=$(echo $SERVER_OUTPUT | jq ".end.streams.receiver.bits_per_second")
             echo "Server-side bandwidth from $CLIENT to $SERVER: $BANDWIDTH bps"

             # # Kill the iperf3 server process
             # docker exec $SERVER pkill iperf3
             # sleep 1
             # # Restart the iperf3 server for the next client
             # docker exec -d $SERVER iperf3 -s
         fi
     done
     docker exec $SERVER pkill iperf3
 done

# #!/bin/sh
#
#
# if [ "$(docker ps | grep "node" | wc -l)" -ne 5 ]; then
#   echo "make sure all containers are running"
#   exit 1
# fi
#
# NODES=("node1" "node2" "node3" "node4" "node5")
# OUTPUT_FILE="./tmp/iperf3_output.json"
#
# for SERVER in "${NODES[@]}"; do
#     # Start iperf3 server on the current node
#     docker exec -d $SERVER iperf3 -s --json > $OUTPUT_FILE
#
#     for CLIENT in "${NODES[@]}"; do
#         if [ "$SERVER" != "$CLIENT" ]; then
#             echo "starting iperf3 client on $CLIENT"
#             docker exec $CLIENT iperf3 -c $SERVER -t 3 -b 60M > /dev/null
#             echo "done iperf3"
#             sleep 3
#
#             SERVER_OUTPUT=$(docker exec $SERVER cat $OUTPUT_FILE)
#             echo "$SERVER_OUTPUT"
#             BANDWIDTH=$(echo $SERVER_OUTPUT | jq ".end.streams[0].receiver.bits_per_second")
#             echo "Server-side bandwidth from $CLIENT to $SERVER: $BANDWIDTH bps"
#
#             # Kill the iperf3 server process
#             docker exec $SERVER pkill iperf3
#             sleep 1
#         fi
#     done
#     docker exec $SERVER rm -f $OUTPUT_FILE
# done
#
