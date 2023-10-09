/* -*- mode: c; coding:utf-8; ##: nil; -*-                          */
/*                                                                  */
/*  FILENAME     :  udp_echo_server.c                               */
/*  DESCRIPTION  :  UDP Echo Server                                 */
/*                                                                  */
/*  DATE         :  Sep. 01, 2020                                   */
/*                                                                  */

#include "icslab2_net.h"
#define MAX_EVENTS 5

int main(int argc, char **argv) {
  int sock;                      /* ソケットディスクリプタ */
  struct sockaddr_in serverAddr; /* サーバ＝自分用アドレス構造体 */
  struct sockaddr_in clientAddr; /* クライアント＝相手用アドレス構造体 */
  unsigned int port = UDP_SERVER_PORT;
  int addrLen;       /* clientAddrのサイズ */
  char buf[BUF_LEN]; /* 受信バッファ */
  int n;             /* 受信バイト数 */
  char *filename;
  int fd;
  int epfd;
  int nfds;
  int rev_cnt = 0;

  struct epoll_event ev, events[MAX_EVENTS];

  struct in_addr addr; /* アドレス表示用 */

  /* コマンドライン引数の処理 */
  if (argc == 2 && strncmp(argv[1], "-h", 2) == 0) {
    printf("Usage: %s\n", argv[0]);
    printf("       default port # %d\n", UDP_SERVER_PORT);
    return 0;
  }

  filename = argv[1];
  printf("filename: %s\n", filename);

  /* STEP 1: UDPソケットをオープンする */
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return 1;
  }

  /* STEP 2: クライアントからの要求を受け付けるIPアドレスとポートを設定する */
  memset(&serverAddr, 0, sizeof(serverAddr)); /* ゼロクリア */
  serverAddr.sin_family = AF_INET;            /* Internetプロトコル */
  serverAddr.sin_port = htons(port);          /* 待ち受けるポート */
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* どのIPアドレス宛でも */
  if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("bind");
    return 1;
  }

  // create epoll
  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.fd = sock;
  epfd = epoll_create(MAX_EVENTS);
  if (epfd < 0) {
    perror("epoll_create");
    return 1;
  }

  // add socket to epoll
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) != 0) {
    perror("epoll_ctl 1");
    return 1;
  }

  for (;;) {
    printf("wait\n");
    // addrLen = sizeof(clientAddr);
    // n = recvfrom(sock, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
    //              (socklen_t *)&addrLen);
    // if (n < 0) {
    //   perror("recvfrom");
    //   break;
    // }
    // printf("received data\n");

    nfds = epoll_wait(epfd, events, MAX_EVENTS, 3000);
    if (nfds < 0) {
      perror("epoll_wait");
      return 1;
    }
    if (nfds == 0) {
      continue;
    }
    printf("got epoll event\n");
    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == sock) {
        addrLen = sizeof(clientAddr);
        n = recvfrom(sock, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
                     (socklen_t *)&addrLen);
        if (n < 0) {
          perror("recvfrom");
        }
        rev_cnt++;
      }
      fd = open(filename, O_RDONLY);
      if (fd < 0) {
        perror("open");
        return 1;
      }

      while ((n = read(fd, buf, BUF_LEN)) > 0) {
        if (sendto(sock, buf, n, 0, (struct sockaddr *)&clientAddr, addrLen) !=
            n) {
          perror("sendto");
          break;
        }
      }

      close(fd);
      // send eof
      // this will send 0 byte packet
      printf("send eof\n");
      if (sendto(sock, buf, 0, 0, (struct sockaddr *)&clientAddr, addrLen) !=
          0) {
        perror("sendto");
        break;
      }
    }

    printf("\n");
  }

  close(sock);

  return 0;
}
/*--------------------------- <end> --------------------------------*/

/* Local Variables: */
/* compile-command: "gcc -g udp_echo_server.c -o udp_echo_server.out " */
/* End: */