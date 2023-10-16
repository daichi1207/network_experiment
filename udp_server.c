/* -*- mode: c; coding:utf-8; ##: nil; -*-                          */
/*                                                                  */
/*  FILENAME     :  udp_echo_server.c                               */
/*  DESCRIPTION  :  UDP Echo Server                                 */
/*                                                                  */
/*  DATE         :  Sep. 01, 2020                                   */
/*                                                                  */

#include "icslab2_net.h"
// adjust this for buffer overflow
#define MAX_EVENTS 5
#define SLEEPTIME 270

#define node2_from3 "172.24.0.20"

int main(int argc, char **argv) {
  int sock;                      /* ソケットディスクリプタ */
  struct sockaddr_in serverAddr; /* サーバ＝自分用アドレス構造体 */
  // struct sockaddr_in serverAddr2; /* サーバ＝自分用アドレス構造体 */
  struct sockaddr_in clientAddr; /* クライアント＝相手用アドレス構造体 */
  struct sockaddr_in sub_clientAddr; /* サブ経路のipアドレス */
  const char *sub_router_ipAddr_str = node2_from3;  // sub routerのipアドレス
  int addrLen;                                      /* clientAddrのサイズ */
  char buf[BUF_LEN];                                /* 受信バッファ */
  int n;                                            /* 受信バイト数 */
  char *filename;
  char *filename_part1 = "part1.dat";
  char *filename_part2 = "part2.dat";
  int fd;
  int fd_part1;
  int fd_part2;
  int epfd;
  int nfds;

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
  file_divide(filename);  // subファイルの作成

  /* STEP 1: UDPソケットをオープンする */
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return 1;
  }
  /* STEP 2: クライアントからの要求を受け付けるIPアドレスとポートを設定する */
  memset(&serverAddr, 0, sizeof(serverAddr)); /* ゼロクリア */
  serverAddr.sin_family = AF_INET;            /* Internetプロトコル */
  serverAddr.sin_port = htons(3000);          /* 待ち受けるポート */
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* どのIPアドレス宛でも */
  /* STEP 3:ソケットとアドレスをbindする */
  if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("bind");
    return 1;
  }

  memset(&sub_clientAddr, 0, sizeof(sub_clientAddr)); /* 0クリア */
  sub_clientAddr.sin_family = AF_INET;   /* Internetプロトコル */
  sub_clientAddr.sin_port = htons(3000); /* 待ち受けるポート */
  sub_clientAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* どのIPアドレス宛でも */
  inet_pton(AF_INET, sub_router_ipAddr_str,
            &sub_clientAddr.sin_addr.s_addr);  // subrouterのipアドレスを設定

  // memset(&serverAddr2, 0, sizeof(serverAddr2)); /* ゼロクリア */
  // serverAddr2.sin_family = AF_INET;             /* Internetプロトコル */
  // serverAddr2.sin_port = htons(3000);           /* 待ち受けるポート */
  // serverAddr2.sin_addr.s_addr = htonl(INADDR_ANY); /*
  // どのIPアドレス宛でも */
  // /* STEP 3:ソケットとアドレスをbindする */
  // if (bind(sock, (struct sockaddr *)&serverAddr2, sizeof(serverAddr2)) <
  // 0) {
  //   perror("bind");
  //   return 1;
  // }

  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.fd = sock;
  epfd = epoll_create(MAX_EVENTS);
  if (epfd < 0) {
    perror("epoll_create");
    return 1;
  }

  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) != 0) {
    perror("epoll_ctl 1");
    return 1;
  }

  /* STEP 5: 受信データをクライアントに送り返す */
  nfds = epoll_wait(epfd, events, MAX_EVENTS, 3000);
  if (nfds < 0) {
    perror("epoll_wait");
    return 1;
  }
  printf("got epoll\n");
  printf("%d\n", nfds);

  for (;;) { /* 無限ループ */
    /* STEP 4: クライアントからのデータグラムを受けとる */
    fd_part1 = open(filename_part1, O_RDONLY);
    if (fd_part1 < 0) {
      perror("open");
      return 1;
    }
    fd_part2 = open(filename_part2, O_RDONLY);
    if (fd_part2 < 0) {
      perror("open");
      return 1;
    }
    printf("waiting connection...\n");
    addrLen = sizeof(clientAddr);
    n = recvfrom(sock, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
                 (socklen_t *)&addrLen);
    if (n < 0) {
      perror("recvfrom");
      break;
    }
    printf("received data\n");

    /* 受信パケットの送信元IPアドレスとポート番号を表示 */
    addr.s_addr = clientAddr.sin_addr.s_addr;
    printf("received from :  ip address: %s, ", inet_ntoa(addr));
    printf("port#: %d\n", ntohs(clientAddr.sin_port));

    // if (nfds == 0) {
    //     printf("timeout\n");
    //     continue;
    // }

    // for (int i = 0; i < nfds; i++) {
    //   if (events[i].data.fd == sock) {
    //     addrLen = sizeof(clientAddr);
    //     n = recvfrom(sock, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
    //                  (socklen_t *)&addrLen);
    //     if (n < 0) {
    //       perror("recvfrom");
    //     }
    //   }
    // }

    printf("start sending\n");
    int sendcomplete_flag_1 = 0;
    int sendcomplete_flag_2 = 0;
    while (1) {
      if (!sendcomplete_flag_1 && (n = read(fd_part1, buf, BUF_LEN)) > 0) {
        if (sendto(sock, buf, n, 0, (struct sockaddr *)&clientAddr, addrLen) !=
            n) {
          perror("sendto");
          break;
        }
      } else {
        printf("part1 sent\n");
        sendcomplete_flag_1 = 1;
      }
      if (!sendcomplete_flag_2 && (n = read(fd_part2, buf, BUF_LEN)) > 0) {
        if (sendto(sock, buf, n, 0, (struct sockaddr *)&sub_clientAddr,
                   addrLen) != n) {
          perror("sendto");
          break;
        }
      } else {
        printf("part2 sent\n");
        sendcomplete_flag_2 = 1;
      }
      if (sendcomplete_flag_1 && sendcomplete_flag_2) break;
      usleep(SLEEPTIME);
    }

    // send eof
    printf("sent eof\n");
    sendto(sock, buf, 0, 0, (struct sockaddr *)&clientAddr, addrLen);
    sendto(sock, buf, 0, 0, (struct sockaddr *)&sub_clientAddr, addrLen);
  }

  close(sock); /* ソケットのクローズ */

  return 0;
}
/*--------------------------- <end> --------------------------------*/

/* Local Variables: */
/* compile-command: "gcc -g udp_echo_server.c -o udp_echo_server.out " */
/* End: */

int file_divide(char *filename) {
  // const char *filename = "test.dat";  // バイナリーデータを格納したファイル名

  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    perror("ファイルのオープンに失敗しました");
    return 1;
  }

  fseek(file, 0, SEEK_END);      // ファイルの末尾に移動
  long file_size = ftell(file);  // ファイルサイズを取得
  rewind(file);                  // ファイルポインタを先頭に戻す

  // ファイルサイズが奇数の場合、分割が均等にならないので注意が必要

  FILE *part1 = fopen("part1.dat", "wb");
  FILE *part2 = fopen("part2.dat", "wb");

  if (part1 == NULL || part2 == NULL) {
    perror("ファイルのオープンに失敗しました");
    return 1;
  }

  // ファイルを2分割する処理
  char buffer;
  for (long i = 0; i < file_size; i++) {
    fread(&buffer, sizeof(char), 1, file);
    if (i < file_size / 2) {
      fwrite(&buffer, sizeof(char), 1, part1);
    } else {
      fwrite(&buffer, sizeof(char), 1, part2);
    }
  }

  fclose(file);
  fclose(part1);
  fclose(part2);

  return 0;
}