// routing_table.h

#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <netinet/in.h> // for struct in_addr

typedef struct {
  struct in_addr src;
  struct in_addr dest;
} RouteEntry;

struct in_addr get_forwarding_address(const struct in_addr src);

#endif // ROUTING_TABLE_H
