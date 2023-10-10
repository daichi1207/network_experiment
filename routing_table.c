// routing_table.c

#include <arpa/inet.h>  
#include "routing_table.h"
#include <string.h>

static RouteEntry routing_table[2];

void init_routing_table(void) {
    routing_table[0].src.s_addr = inet_addr("172.22.0.10");
    routing_table[0].dest.s_addr = inet_addr("172.27.0.30");
    routing_table[1].src.s_addr = inet_addr("172.27.0.30");
    routing_table[1].dest.s_addr = inet_addr("172.22.0.10");
}

struct in_addr get_forwarding_address(const struct in_addr src) {
  for (int i = 0; i < sizeof(routing_table) / sizeof(RouteEntry); i++) {
    if (memcmp(&src, &routing_table[i].src, sizeof(struct in_addr)) == 0) {
      return routing_table[i].dest;
    }
  }

  struct in_addr null_addr;
  null_addr.s_addr = INADDR_NONE;
  return null_addr;
}
