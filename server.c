#include "server.h"
#include <fcntl.h>
#include <pthread.h>

/* --------- Function Prototypes --------- */
void *server_listen(void *arg);
void *server_respond(void *arg);
void error(const char *msg);
/* ------- End Function Prototypes ------- */

server_data_t svr_data;

int main(int argc, char *argv[])
{
  pthread_t thrListen, thrRespond;
  void *thrListen_ret;
  void *thrRespond_ret;

  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  svr_data.sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (svr_data.sockfd < 0)
  {
    error("ERROR opening socket");
  }

  memset((char *)&svr_data.serv_addr, 0, sizeof(svr_data.serv_addr));
  svr_data.portno = atoi(argv[1]);

  svr_data.serv_addr.sin_family = AF_INET;
  svr_data.serv_addr.sin_addr.s_addr = INADDR_ANY;
  svr_data.serv_addr.sin_port = htons(svr_data.portno);

  if (bind(svr_data.sockfd, (struct sockaddr *)&svr_data.serv_addr, sizeof(svr_data.serv_addr)) < 0)
  {
    error("Binding Failed.");
  }
  listen(svr_data.sockfd, 5);
  svr_data.clilen = sizeof(svr_data.cli_addr);

  svr_data.newsockfd = accept(svr_data.sockfd, (struct sockaddr *)&svr_data.cli_addr, &svr_data.clilen);
  int flags = fcntl(svr_data.newsockfd, F_GETFL, 0);
  fcntl(svr_data.newsockfd, F_SETFL, flags | O_NONBLOCK);

  if (svr_data.newsockfd < 0)
  {
    error("Error on Accept");
  }

  svr_data.tx_stop = false;

  pthread_create(&thrListen, NULL, server_listen, &svr_data);
  pthread_create(&thrRespond, NULL, server_respond, &svr_data);

  // Wait for the threads to finish
  pthread_join(thrListen, &thrListen_ret);
  pthread_join(thrRespond, &thrRespond_ret);

  close(svr_data.newsockfd);
  close(svr_data.sockfd);
  return 0;
}

void *server_listen(void *arg)
{
  server_data_t *svr_dat = (server_data_t *)arg;

  int n;
  while (1)
  {
    memset(svr_dat->rx_buff, 0, sizeof(svr_dat->rx_buff));
    n = read(svr_dat->newsockfd, svr_dat->rx_buff, 255);
    if (n > 0)
    {
      printf("Client: %s\n", svr_dat->rx_buff);
    }

    /* Stop condition for server */
    int l = strncmp("Bye", svr_dat->rx_buff, 3);
    if (0 == l)
    {
      svr_dat->tx_stop = true;
      break;
    }
  }
  return (void *)0;
}

void *server_respond(void *arg)
{
  server_data_t *svr_dat = (server_data_t *)arg;

  int n;
  while (1)
  {
    /* Wait for user-input into the tx-buffer */
    fgets(svr_dat->tx_buff, 255, stdin);

    n = write(svr_dat->newsockfd, svr_dat->tx_buff, strlen(svr_dat->tx_buff));
    if (n < 0)
    {
      error("ERROR on Writing.");
    }

    /* Stop condition, set by `server_listen` */
    if (true == svr_dat->tx_stop)
    {
      break;
    }
  }
  return (void *)0;
}

void error(const char *msg)
{
  perror(msg);
  exit(1);
}
