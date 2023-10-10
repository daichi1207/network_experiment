#include "icslab2_net.h"

#define MAX_EVENTS 5

int main(int argc, char *argv[]) {
    int recv_sock, send_sock;
    struct sockaddr_in recv_addr, send_addr;
    char buf[BUF_SIZE];
    int epfd;
    int nfds;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];

    // 受け取り用ソケット
    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(3000); // 
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(recv_sock, (struct sockaddr*) &recv_addr, sizeof(recv_addr));

    // 送信用ソケット
    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(3001); //
    send_addr.sin_addr.s_addr = htonl(INADDR_ANY); //

    // epollインスタンスを作成
    epfd = epoll_create(MAX_EVENTS)
    if (epfd < 0){
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    // epollインスタンスにrecv_sockを登録
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = recv_sock;

    epoll_ctl(epfd, EPOLL_CTL_ADD, recv_sock, &ev)

    // メインのところ
    nfds = epoll_wait(epfd, events, MAX_EVENTS, 3000);
    if (nfds == 0){
      continue;
    }

    for (int i; i < nfds; i++){
      if (event[i].data.fd == recv_sock){
        int str_len = recvfrom(recv_sock, buf, BUF_SIZE, 0, NULL, 0);

        // todo udp headerを見て、送信先を決める

        // 受信したデータをNode1に転送
        sendto(send_sock, buf, str_len, 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
      }
    }

    close(recv_sock);
    close(send_sock);
    return 0;
}

/*
// #define BUF_SIZE 1024

// int main(int argc, char *argv[]) {
//     int recv_sock, send_sock;
//     struct sockaddr_in recv_addr, send_addr;
//     char buf[BUF_SIZE];
//     socklen_t addr_size;

//     // Node3からデータを受信するためのソケット
//     recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
//     memset(&recv_addr, 0, sizeof(recv_addr));
//     recv_addr.sin_family = AF_INET;
//     recv_addr.sin_addr.s_addr = htonl(); //
//     recv_addr.sin_port = htons(); //

//     if (bind(recv_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1) {
//         perror("bind() error");
//         close(recv_sock);
//         exit(1);
//     }

//     // Node1にデータを送信するためのソケット
//     send_sock = socket(PF_INET, SOCK_DGRAM, 0);
//     memset(&send_addr, 0, sizeof(send_addr));
//     send_addr.sin_family = AF_INET;
//     send_addr.sin_addr.s_addr = inet_addr("192.168.1.1"); // Node1のIPアドレス
//     send_addr.sin_port = htons(); //

//     while (1) {
//         int str_len = recvfrom(recv_sock, buf, BUF_SIZE, 0, NULL, 0);
//         if (str_len < 0) {
//             break;
//         }

//         // 受信したデータをNode1に転送
//         sendto(send_sock, buf, str_len, 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
//     }

//     close(recv_sock);
//     close(send_sock);
//     return 0;
// }
*/