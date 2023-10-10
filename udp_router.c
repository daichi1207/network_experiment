#include "icslab2_net.h"
#include "routing_table.h"

#define BUF_SIZE 1024
#define MAX_EVENTS 5

int main(int argc, char *argv[]) {
  int recv_sock;
  struct sockaddr_in recv_addr, send_addr;
  struct sockaddr_in from_addr;
  socklen_t addr_len = sizeof(from_addr);
  char buf[BUF_SIZE];
  int epfd;
  int nfds;
  struct epoll_event ev;
  struct epoll_event events[MAX_EVENTS];

  // Receiving socket
  recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&recv_addr, 0, sizeof(recv_addr));
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_port = htons(3000);
  recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(recv_sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr));

  // epoll instance creation
  epfd = epoll_create(MAX_EVENTS);
  if (epfd < 0) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }

  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.fd = recv_sock;
  epoll_ctl(epfd, EPOLL_CTL_ADD, recv_sock, &ev);

  while (1) {
    nfds = epoll_wait(epfd, events, MAX_EVENTS, 3000);
    if (nfds == 0) {
      continue;
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == recv_sock) {
        int str_len = recvfrom(recv_sock, buf, BUF_SIZE, 0,
                               (struct sockaddr *)&from_addr, &addr_len);

        struct in_addr forward_addr =
            get_forwarding_address(from_addr.sin_addr);
        if (forward_addr.s_addr != INADDR_NONE) {
          send_addr.sin_family = AF_INET;
          send_addr.sin_port = htons(3001);
          send_addr.sin_addr = forward_addr;
          sendto(recv_sock, buf, str_len, 0, (struct sockaddr *)&send_addr,
                 sizeof(send_addr));
        }
      }
    }
  }

  close(recv_sock);
  return 0;
}
