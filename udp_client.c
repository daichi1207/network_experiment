/* -*- mode: c; coding:utf-8; ##: nil; -*-                          */
/*                                                                  */
/*  FILENAME     :  udp_echo_client_ep.c                             */
/*  DESCRIPTION  :  UDP echo client                                 */
/*                  ホスト名の解決を含まない。IPアドレスで指定する。     */
/*                  epollを使った多重化                            */
/*  DATE         :  Sep. 01, 2020                                   */
/*  UPDATE       :                                                  */
/*                                                                  */

/*-------------------------- <include>  ----------------------------*/
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "icslab2_net.h"

#define NEVENTS 16

/*-------------------------- <functions> ---------------------------*/

int main(int argc, char **argv) {
  char *server_ipaddr_str = "127.0.0.1"; /* サーバIPアドレス（文字列） */
  unsigned int port = 3000;              /* ポート番号（文字列） */
  char *filename = NULL;
  int fd = 0;
  int fd_part1 = 0;
  int fd_part2 = 0;

  int sock1, sock2;               /* ソケットディスクリプタ */
  struct sockaddr_in serverAddr1; /* サーバ＝相手用のアドレス構造体 */
  struct sockaddr_in serverAddr2; /* サーバ＝相手用のアドレス構造体 */
  struct sockaddr_in clientAddr; /* クライアント＝自分用のアドレス構造体 */
  struct sockaddr_in sub_clientAddr; /* サブ経路のipアドレス */
  int addrLen;                       /* serverAddrのサイズ */
  char buf[BUF_LEN];                 /* 受信バッファ */
  int n;                             /* 読み込み／受信バイト数 */

  struct in_addr addr1; /* アドレス表示用 */
  struct in_addr addr2; /* アドレス表示用 */

  int epfd; /* epollインスタンスのファイルデスクリプタ */
  struct epoll_event ev, events[NEVENTS]; /* イベントとイベントの配列 */
  int nfds; /* 要求するI/Oが可能なｆｄの数 */
  int i;

  int cnt = 0; /* timeoutのカウンタ用 */

  int sock_begin_flag = 0;  // sock通信の開始フラグ
  int bflag = 0;            /*終了フラグ*/
  // const char EOF_SIGNAL[] = "END_OF_TRANSMISSION";

  /*スループット計測用*/
  unsigned int sec;
  int nsec;
  double time;
  struct timespec start, end;
  int rev_cnt = 0;

  /* コマンドライン引数の処理 */
  if (argc == 2 && strncmp(argv[1], "-h", 2) == 0) {
    printf("Usage: %s [dst_ip_addr] [port] [o_filename]\n", argv[0]);
    return 1;
  }
  if (argc > 1) /* 宛先を指定のIPアドレスにする。 portはデフォルト */
    server_ipaddr_str = argv[1];
  if (argc > 2) /* 宛先を指定のIPアドレス、portにする */
    port = (unsigned int)atoi(argv[2]);
  if (argc > 3) { /*出力ファイルの設定*/
    printf("output file is %s\n", argv[3]);
    filename = argv[3];
    fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
      perror("open");
      return 1;
    }
    fd_part1 = open("part1_out.dat", O_WRONLY | O_CREAT, 0644);
    if (fd_part1 < 0) {
      perror("open");
      return 1;
    }
    fd_part2 = open("part2_out.dat", O_WRONLY | O_CREAT, 0644);
    if (fd_part2 < 0) {
      perror("open");
      return 1;
    }
  }

  /* STEP 1: 宛先サーバのIPアドレスとポートを指定する */
  memset(&serverAddr1, 0, sizeof(serverAddr1)); /* 0クリア */
  serverAddr1.sin_family = AF_INET;             /* Internetプロトコル */
  serverAddr1.sin_port = htons(3000); /* サーバの待受ポート */
  serverAddr1.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(&serverAddr2, 0, sizeof(serverAddr2)); /* 0クリア */
  serverAddr2.sin_family = AF_INET;             /* Internetプロトコル */
  serverAddr2.sin_port = htons(3001); /* サーバの待受ポート */
  serverAddr2.sin_addr.s_addr = htonl(INADDR_ANY);

  /* IPアドレス（文字列）から変換 */
  inet_pton(AF_INET, server_ipaddr_str, &serverAddr1.sin_addr.s_addr);
  inet_pton(AF_INET, server_ipaddr_str, &serverAddr2.sin_addr.s_addr);

  /* 確認用：IPアドレスを文字列に変換して表示 */
  addr1.s_addr = serverAddr1.sin_addr.s_addr;
  printf("ip address: %s\n", inet_ntoa(addr1));
  printf("port#: %d\n", ntohs(serverAddr1.sin_port));
  addr2.s_addr = serverAddr2.sin_addr.s_addr;
  printf("ip address: %s\n", inet_ntoa(addr2));
  printf("port#: %d\n", ntohs(serverAddr2.sin_port));

  /* STEP 2: UDPソケットをオープンする */
  sock1 = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock1 < 0) {
    perror("socket");
    return 1;
  }
  sock2 = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock2 < 0) {
    perror("socket");
    return 1;
  }

  /* クライアントからの要求を受け付けるIPアドレスとポートを設定する */
  memset(&clientAddr, 0, sizeof(clientAddr)); /* ゼロクリア */
  clientAddr.sin_family = AF_INET;            /* Internetプロトコル */
  clientAddr.sin_port = htons(3000);          /* 待ち受けるポート */
  clientAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* どのIPアドレス宛でも */
  /* STEP 3:ソケットとアドレスをbindする */
  if (bind(sock1, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
    perror("bind");
    return 1;
  }
  memset(&sub_clientAddr, 0, sizeof(sub_clientAddr)); /* ゼロクリア */
  sub_clientAddr.sin_family = AF_INET;   /* Internetプロトコル */
  sub_clientAddr.sin_port = htons(3001); /* 待ち受けるポート */
  sub_clientAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* どのIPアドレス宛でも */
  /* STEP 3:ソケットとアドレスをbindする */
  if (bind(sock2, (struct sockaddr *)&sub_clientAddr, sizeof(sub_clientAddr)) <
      0) {
    perror("bind");
    return 1;
  }

  /* 多重化の準備 */
  /* STEP 3: epollインスタンスの作成 */
  epfd = epoll_create(NEVENTS);
  if (epfd < 0) {
    perror("epoll_create");
    return 1;
  }

  /* STEP 4': 監視するfdとイベントを登録 */
  /* sockを登録 */
  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.fd = sock1;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock1, &ev) != 0) {
    perror("epoll_ctl 1");
    return 1;
  }
  ev.data.fd = sock2;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock2, &ev) != 0) {
    perror("epoll_ctl 1");
    return 1;
  }
  // if (epoll_ctl_add_in(epfd, fd_part1) != 0) {
  //   perror("epoll_ctl 1");
  //   return 1;
  // }
  // if (epoll_ctl_add_in(epfd, fd_part2) != 0) {
  //   perror("epoll_ctl 1");
  //   return 1;
  // }

  /*ダミーのファイル要求メッセージ*/
  //   sprintf(buf, "GET %s\r\n", dummy_file);
  if (rev_cnt == 0) {
    printf("clock start");
    clock_gettime(CLOCK_REALTIME, &start);
  }
  sprintf(buf, "GET %s\r\n", "dummy1");
  sendto(sock1, buf, strlen(buf), 0, (struct sockaddr *)&serverAddr1,
         sizeof(serverAddr1));
  // sendto(sock2, buf, strlen(buf), 0, (struct sockaddr *)&serverAddr2,
  //        sizeof(serverAddr2));
  printf("Send %ld bytes data: %s", strlen(buf), buf);
  while (1) {
    /* STEP 5: イベント発生をを待つ */
    /* 最後の引数はtimeout (msec単位), -1はtimeoutなし = ブロック */
    /* nfdsには処理が可能になったイベント数が返される             */
    /* events[]には処理可能なイベントとデスクリプタが登録されている */
    nfds = epoll_wait(epfd, events, NEVENTS, 100);

    if (bflag == 1) {
      printf("done connection\n");
      break;
    }
    if (nfds < 0) {
      perror("epoll_wait");
      return 1;
    }
    /* timeout */
    if (nfds == 0) {
      if (sock_begin_flag == 1) {
        bflag = 1;
        sock_begin_flag = 0;
      } else {
        printf("timeout %d\n", cnt++);
        continue;
      }
    } else {  // socketのepollが立った場合
      /* STEP 6: events[]を順次確認して必要な処理を行う */
      for (i = 0; i < nfds; i++) {
        // int current_sock = events[i].data.fd;
        addrLen = sizeof(clientAddr);
        // n = recvfrom(sock1, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
        //              (socklen_t *)&addrLen);
        if (n < 0) {
          perror("recvfrom");
        }
        if (events[i].data.fd == sock2) {
          // printf("data received from sock2\n");
          n = recvfrom(sock2, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
                       (socklen_t *)&addrLen);
          write(fd_part2, buf, n);

          // if (n1 < 0) {
          //   perror("recvfrom");
          // }
          // if (n1 == 0) {
          //   printf("all data have saved\n");
          //   bflag = 1;  // 最後はn=0じゃないことがある。意味ないかもしれない
          //   break;
          // } else {
          //   // printf("data saving, n=%d\n", n);
          //   write(fd, buf, n1);
          //   write(fd, buf, n2);
          //   rev_cnt++;
          // }
        }
        if (events[i].data.fd == sock1) {
          /* STEP 8: sockに関するイベントなら */
          /* サーバからデータグラムを受けとる */
          sock_begin_flag = 1;
          // printf("data received from sock1\n");
          // addrLen = sizeof(clientAddr);
          n = recvfrom(sock1, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
                       (socklen_t *)&addrLen);
          write(fd_part1, buf, n);
        }
      }
    }
    /* printf("\n"); */
  }

  clock_gettime(CLOCK_REALTIME, &end);
  printf("\nrev count is %d\n", rev_cnt);
  sec = end.tv_sec - start.tv_sec;
  nsec = end.tv_nsec - start.tv_nsec;
  time = (double)sec + (double)nsec * 1e-9;
  printf("time; %lf\n", time);
  printf("total time is %lf ms\n", time * 1e3);
  // printf("throughput is %lf Mbps\n", (double)rev_cnt*BUF_LEN*8/time/1e6);
  printf("throughput is %lf Mbps\n", 104.8576 * 8 / time);

  close(sock1);
  close(sock2);

  return 0;
}

/* Local Variables: */
/* compile-command: "gcc -g udp_echo_client_ep.c -o udp_echo_client_ep.out" */
/* End: */
