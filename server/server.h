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

#define MAX_CLIENT_NUM (20) /* Maximum number of clients that can connect to the server */

typedef enum{
  UNUSED = 0,
  RUNNING,
  WAITING,
  STOPPING, /* task complete, waiting to be garbage collected */
  JOINED,
}thr_state_t;

typedef struct server_conn
{
  int clntSockFd;

  struct pollfd pfd;

  thr_state_t thr_state;
  pthread_t thrListen;
  pthread_t thrRespond;
  char tx_buff[500];
  char rx_buff[500];
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