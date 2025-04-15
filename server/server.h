#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

typedef struct server_data
{
  /* Listen */
  char rx_buff[255];

  /* Respond */
  char tx_buff[255];
  bool tx_stop;

  /* Global */
  int sockfd;    /* Main socket? */
  int newsockfd; /* Client socket? */
  int portno;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  socklen_t clilen;

} server_data_t;

#endif /* SERVER_H_ */