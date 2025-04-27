#ifndef SERVER_H_
#define SERVER_H_

extern "C"{
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <poll.h>
}
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <thread>

#define MAX_CLIENT_NUM (20) /* Maximum number of clients that can connect to the server */
#define RX_BUFF_SIZE (5094)
#define TX_BUFF_SIZE (5094)

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
  int my_idx;

  thr_state_t thr_state;
  pthread_t thrListen;
  pthread_t thrRespond;
  char tx_buff[TX_BUFF_SIZE];
  char rx_buff[RX_BUFF_SIZE];
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