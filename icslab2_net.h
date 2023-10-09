/* -*- mode: c; coding: utf-8-unix; ##: nil; -*-                    */
/*                                                                  */
/*  FILENAME     :  ics_lab2_net.h                                  */
/*  DESCRIPTION  :  UDP/TCP common header                           */
/*  DATE         :  Sep. 01, 2020                                   */
/*  UPDATE       :                                                  */
/*                                                                  */
/*-------------------------- <include>  ----------------------------*/

#ifndef ICSLAB2_NET_H
#define ICSLAB2_NET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
*/

/*-------------------------- <define>   ----------------------------*/
/* UDP Echo Serverのポート番号 */
#define UDP_SERVER_PORT     (unsigned short)10000
#define UDP_SERVER_PORT_STR "10000"
/* TCP Echo Serverのポート番号 */
#define TCP_SERVER_PORT     (unsigned short)10000
#define TCP_SERVER_PORT_STR  "10000"
/* Server名 暫定的に自ホストとしている */
#define SERVER_HOSTNAME     "localhost"
#define SERVER_IP_ADDR_STR  "127.0.0.1"
/* バッファ長 */
#define BUF_LEN             1000

#endif
