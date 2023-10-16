// routing_table.c

#include "routing_table.h"

#include <arpa/inet.h>
#include <string.h>

static RouteEntry routing_table[6];
#define node1_from4 "172.22.0.10"
#define node3_from4 "172.27.0.30"

#define node1_from5 "172.23.0.10"
#define node2_from5 "172.26.0.20"

#define node5_from2 "172.26.0.50"
#define node3_from2 "172.24.0.30"

void init_routing_table(void) {
  /*node4*/
  routing_table[0].src.s_addr =
      inet_addr(node1_from4);  // 1->4->3. node1からnode3への転送先
  routing_table[0].dest.s_addr =
      inet_addr(node3_from4);  // 1->4->3. node1からnode3への転送先
  routing_table[1].src.s_addr = inet_addr(node3_from4);
  // 1->4->3. node3からnode1への転送先
  routing_table[1].dest.s_addr = inet_addr(node1_from4);
  // 1->4->3. node3からnode1への転送先

  /*node5*/
  routing_table[2].src.s_addr =
      inet_addr(node1_from5);  // 1->5->2->3. node1からnode2への転送先
  routing_table[2].dest.s_addr =
      inet_addr(node2_from5);  // 1->5->2->3. node1からnode2への転送先
  routing_table[3].src.s_addr = inet_addr(node2_from5);
  // 1->4->3. node2からnode1への転送先
  routing_table[3].dest.s_addr = inet_addr(node1_from5);
  // 1->4->3. node2からnode1への転送先

  /*node2*/
  routing_table[4].src.s_addr =
      inet_addr(node5_from2);  // 1->5->2->3. node5からnode3への転送先
  routing_table[4].dest.s_addr =
      inet_addr(node3_from2);  // 1->5->2->3. node5からnode3への転送先
  routing_table[5].src.s_addr = inet_addr(node3_from2);
  // 1->4->3. node5からnode3への転送先
  routing_table[5].dest.s_addr = inet_addr(node5_from2);
  // 1->4->3. node5からnode3への転送先
}

struct in_addr get_forwarding_address(const struct in_addr src) {
  for (int i = 0; i < sizeof(routing_table) / sizeof(RouteEntry); i++) {
    if (memcmp(&src, &routing_table[i].src, sizeof(struct in_addr)) == 0) {
      // printf("get_forwarding_address: %s\n",
      // inet_ntoa(routing_table[i].dest));
      return routing_table[i].dest;
    }
  }

  struct in_addr null_addr;
  null_addr.s_addr = INADDR_NONE;
  return null_addr;
}
