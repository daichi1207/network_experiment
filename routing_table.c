// routing_table.c

#include "routing_table.h"
#include <string.h>

static RouteEntry routing_table[] = {
    {.src = {.s_addr = inet_addr("172.20.0.10")},
     .dest = {.s_addr = inet_addr("172.21.0.20")}},
    // add more
};

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
