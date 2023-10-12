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
  unsigned int port = 3000; /* ポート番号（文字列） */
  char *filename = NULL;
  int fd = 1;

  int sock;                      /* ソケットディスクリプタ */
  struct sockaddr_in serverAddr; /* サーバ＝相手用のアドレス構造体 */
  struct sockaddr_in clientAddr; /* クライアント＝自分用のアドレス構造体 */
  int addrLen;                   /* serverAddrのサイズ */
  char buf[BUF_LEN]; /* 受信バッファ */
  int n;             /* 読み込み／受信バイト数 */

  struct in_addr addr; /* アドレス表示用 */

  int epfd; /* epollインスタンスのファイルデスクリプタ */
  struct epoll_event ev, events[NEVENTS]; /* イベントとイベントの配列 */
  int nfds; /* 要求するI/Oが可能なｆｄの数 */
  int i;

  int cnt = 0; /* timeoutのカウンタ用 */

  int bflag = 0; /*終了フラグ*/
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
  }

  /* STEP 1: 宛先サーバのIPアドレスとポートを指定する */
  memset(&serverAddr, 0, sizeof(serverAddr)); /* 0クリア */
  serverAddr.sin_family = AF_INET;            /* Internetプロトコル */
  serverAddr.sin_port = htons(3000);          /* サーバの待受ポート */
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  /* IPアドレス（文字列）から変換 */
  inet_pton(AF_INET, server_ipaddr_str, &serverAddr.sin_addr.s_addr);

  /* 確認用：IPアドレスを文字列に変換して表示 */
  addr.s_addr = serverAddr.sin_addr.s_addr;
  printf("ip address: %s\n", inet_ntoa(addr));
  printf("port#: %d\n", ntohs(serverAddr.sin_port));

  /* STEP 2: UDPソケットをオープンする */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    return 1;
  }

  /* ここで、ローカルでsocketをbind()してもよいが省略する */

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
  ev.data.fd = sock;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) != 0) {
    perror("epoll_ctl 2");
    return 1;
  }

  /*ダミーのファイル要求メッセージ*/
  //   sprintf(buf, "GET %s\r\n", dummy_file);
  sprintf(buf, "GET %s\r\n", "dummy");
  sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&serverAddr,
         sizeof(serverAddr));
  printf("Send %d bytes data: %s", strlen(buf), buf);

  for (;;) {
    /* STEP 5: イベント発生をを待つ */
    /* 最後の引数はtimeout (msec単位), -1はtimeoutなし = ブロック */
    /* nfdsには処理が可能になったイベント数が返される             */
    /* events[]には処理可能なイベントとデスクリプタが登録されている */
    nfds = epoll_wait(epfd, events, NEVENTS, 5000);
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
      printf("timeout %d\n", cnt++);
      continue;
    }

    if (rev_cnt == 0) {
      printf("clock start");
      clock_gettime(CLOCK_REALTIME, &start);
    }
    /* STEP 6: events[]を順次確認して必要な処理を行う */
    for (i = 0; i < nfds; i++) {
      if (events[i].data.fd == sock) {
        /* STEP 8: sockに関するイベントなら */

        /* サーバからデータグラムを受けとる */
        addrLen = sizeof(clientAddr);
        n = recvfrom(sock, buf, BUF_LEN, 0, (struct sockaddr *)&clientAddr,
                     (socklen_t *)&addrLen);
	printf("data received\n");
        if (n < 0) {
          perror("recvfrom");
        }
        if (n == 0) {
          bflag = 1;
        }
        write(fd, buf, n);
        rev_cnt++;
      }
    }
    if (bflag == 1) {
      printf("done connection\n");
      break;
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
  printf("throughput is %lf Mbps\n", 8 / time);

  close(sock);

  return 0;
}

/* Local Variables: */
/* compile-command: "gcc -g udp_echo_client_ep.c -o udp_echo_client_ep.out" */
/* End: */
