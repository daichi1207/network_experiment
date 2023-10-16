#include "icslab2_net.h"
#include "routing_table.h"

#define BUF_SIZE 1000
#define MAX_EVENTS 5

int main(int argc, char *argv[]) {
  int sockets[MAX_EVENTS]; // array to hold socket descriptors
  int socket_count = 1;    // keep track of the number of sockets
  struct sockaddr_in recv_addr, send_addr;
  struct sockaddr_in from_addr;
  socklen_t addr_len = sizeof(from_addr);
  char buf[BUF_SIZE];
  int epfd;
  int nfds;
  struct epoll_event ev;
  struct epoll_event events[MAX_EVENTS];

  init_routing_table();

  // Receiving socket
  sockets[0] =
      socket(PF_INET, SOCK_DGRAM, 0); // store recv_sock in sockets array
  memset(&recv_addr, 0, sizeof(recv_addr));
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_port = htons(3000);
  recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sockets[0], (struct sockaddr *)&recv_addr, sizeof(recv_addr));

  // epoll instance creation
  epfd = epoll_create(MAX_EVENTS);
  if (epfd < 0) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < socket_count; i++) {
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLOUT; // Uncomment if you want to handle
    ev.data.fd = sockets[i];
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockets[i], &ev);
  }

  while (1) {
    nfds = epoll_wait(epfd, events, MAX_EVENTS, 3000);
    if (nfds == 0) {
      continue;
    }
    for (int i = 0; i < nfds; i++) {
      int curr_sock = events[i].data.fd;
      if (curr_sock == sockets[0]) {
        int str_len = recvfrom(curr_sock, buf, BUF_SIZE, 0,
                               (struct sockaddr *)&from_addr, &addr_len);
        if (str_len == 0) {
          printf("receive end packet\n");
          break;
        }
        struct in_addr forward_addr =
            get_forwarding_address(from_addr.sin_addr);
        if (forward_addr.s_addr != INADDR_NONE) {
          send_addr.sin_family = AF_INET;
          send_addr.sin_port = htons(3000);
          send_addr.sin_addr = forward_addr;
          sendto(curr_sock, buf, str_len, 0, (struct sockaddr *)&send_addr,
                 sizeof(send_addr));
        } else {
          printf("dest address not found\n");
          break;
        }
      } else {
      }
    }
  }
  return 0;
}
