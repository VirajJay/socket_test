#include "client.h"
#include <fcntl.h>
#include <pthread.h>

/* --------- Function Prototypes --------- */
void *client_listen(void *arg);
void *client_respond(void *arg);
void error(const char *msg);
/* ------- End Function Prototypes ------- */

client_data_t clnt_data;

int main(int argc, char *argv[])
{

  pthread_t thrListen, thrRespond;
  void *thrListen_ret;
  void *thrRespond_ret;

  if (argc < 3)
  {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  clnt_data.portno = atoi(argv[2]);
  clnt_data.sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (clnt_data.sockfd < 0)
  {
    error("ERROR opening socket");
  }

  clnt_data.server = gethostbyname(argv[1]);
  printf("h_length: %d\n", clnt_data.server->h_length);
  if (NULL == clnt_data.server)
  {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  memset(&clnt_data.serv_addr, 0, sizeof(clnt_data.serv_addr));
  clnt_data.serv_addr.sin_family = AF_INET;
  memcpy((char *)&clnt_data.serv_addr.sin_addr.s_addr, (char *)clnt_data.server->h_addr_list[0], clnt_data.server->h_length);
  clnt_data.serv_addr.sin_port = htons(clnt_data.portno);
  if (connect(clnt_data.sockfd, (struct sockaddr *)&clnt_data.serv_addr, sizeof(clnt_data.serv_addr)) < 0)
  {
    error("ERROR connecting");
  }

  pthread_create(&thrListen, NULL, client_listen, &clnt_data);
  pthread_create(&thrRespond, NULL, client_respond, &clnt_data);

  // Wait for the threads to finish
  pthread_join(thrListen, &thrListen_ret);
  pthread_join(thrRespond, &thrRespond_ret);

  close(clnt_data.sockfd);
  return 0;
}

void *client_listen(void *arg)
{

  client_data_t *cli_dat = (client_data_t *)arg;
  int n;

  while (1)
  {
    memset(cli_dat->rx_buff, 0, sizeof(cli_dat->rx_buff));
    n = read(cli_dat->sockfd, cli_dat->rx_buff, 255);
    if (n < 0)
    {
      error("ERROR reading from socket");
    }
    printf("Server: %s\n", cli_dat->rx_buff);
    int i = strncmp("Bye", cli_dat->rx_buff, 3);
    if (i == 0)
    {
      cli_dat->tx_stop = true;
      break;
    }
  }

  return (void *)0;
}

void *client_respond(void *arg)
{

  client_data_t *cli_dat = (client_data_t *)arg;
  int n;

  while (1)
  {

    memset(cli_dat->tx_buff, 0, sizeof(cli_dat->tx_buff));
    fgets(cli_dat->tx_buff, 255, stdin);
    n = write(cli_dat->sockfd, cli_dat->tx_buff, strlen(cli_dat->tx_buff));

    if (n < 0)
    {
      error("ERROR writing to socket");
    }

    if (true == cli_dat->tx_stop)
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
