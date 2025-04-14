#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>

typedef struct client_data
{
  /* Listen */
  char rx_buff[256];

  /* Respond */
  char tx_buff[256];
  bool tx_stop;

  /* Global */
  int sockfd;
  int portno;
  struct sockaddr_in serv_addr;
  struct hostent *server;

} client_data_t;

#endif /* CLIENT_H_ */