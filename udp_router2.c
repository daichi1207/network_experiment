#include "icslab2_net.h"
#include "routing_table.h"

#define BUF_SIZE 1024
#define MAX_EVENTS 5
#define SLEEPTIME 0

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
  int port_num = 0;
  long sum_bytes = 0;
  long sum_sent_bytes = 0;

  init_routing_table();
  if (argc > 1) /* port */
    port_num = atoi(argv[1]);
  printf("using port num: %d\n", port_num);  // 3000, 3001

  // Receiving socket
  recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&recv_addr, 0, sizeof(recv_addr));
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_port = htons(port_num);
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

  int packet_num = 0;
  while (1) {
    nfds = epoll_wait(epfd, events, MAX_EVENTS, 3000);
    if (nfds == 0) {
      continue;
    }
    // printf("data received\n");
    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == recv_sock) {
        int str_len = recvfrom(recv_sock, buf, BUF_SIZE, 0,
                               (struct sockaddr *)&from_addr, &addr_len);
        sum_bytes += str_len;
        struct in_addr forward_addr = get_forwarding_address(
            from_addr.sin_addr);  // routing_table.cから取得した、転送先アドレス
        if (str_len == 0) {  // 0byteの終端パケットの場合
          printf("receive end packet\n");
          printf("total bytes: %ld\n", sum_bytes);
          printf("total sent bytes: %ld\n", sum_sent_bytes);
          sum_bytes = 0;
          sum_sent_bytes = 0;
          packet_num = 0;
          send_addr.sin_family = AF_INET;
          send_addr.sin_port = htons(port_num);
          send_addr.sin_addr = forward_addr;
          sum_sent_bytes +=
              sendto(recv_sock, buf, str_len, 0, (struct sockaddr *)&send_addr,
                     sizeof(send_addr));
          break;
        }
        if (forward_addr.s_addr != INADDR_NONE) {
          // printf("(%d): send to %s :: data from %s\n", packet_num++,
          //        inet_ntoa(forward_addr), inet_ntoa(from_addr.sin_addr));
          send_addr.sin_family = AF_INET;
          send_addr.sin_port = htons(port_num);
          send_addr.sin_addr = forward_addr;
          sum_sent_bytes +=
              sendto(recv_sock, buf, str_len, 0, (struct sockaddr *)&send_addr,
                     sizeof(send_addr));
          //   usleep(SLEEPTIME);
        } else {
          printf("dest address not found\n");
          break;
        }
      }
    }
  }
  return 0;
}
