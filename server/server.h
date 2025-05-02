#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <poll.h>

#include "http_handler.h"

#define MAX_CLIENT_NUM (20) /* Maximum number of clients that can connect to the server */
#define RX_BUFF_SIZE (5094)
#define TX_BUFF_SIZE (5094)
#define FILE_SIZE_MAX (200U*1024)

typedef enum{
  UNUSED = 0,
  RUNNING,
  WAITING,
  STOPPING, /* task complete, waiting to be garbage collected */
  JOINED,
}thr_state_t;

typedef enum{
  E_OK=0,
  E_NOT_OK,
}Std_Return;

typedef struct server_conn
{
  int clntSockFd;

  struct pollfd pfd;
  int my_idx;

  http_parser httpprsr;
  http_parser_settings httpprsr_settings;
  char url[1024];

  thr_state_t thr_state;
  pthread_t thrSvr;
  char tx_buff[TX_BUFF_SIZE];
  char rx_buff[RX_BUFF_SIZE];

  char response_body_template[FILE_SIZE_MAX]; /* Before compressing */
  unsigned char response_body[FILE_SIZE_MAX]; /* After compressing */
}server_conn_t;

typedef struct server_data
{
  /* Client instances */
  server_conn_t svrConn[MAX_CLIENT_NUM];

  /* Main Server */
  int sockFd;    /* Main socket */
  int currClntIdx;
  int portNo;
  struct sockaddr_in servAddr;
  struct sockaddr_in clntAddr;
  socklen_t clntLen;
} server_data_t;

#endif /* SERVER_H_ */