#include "server.h"
#include <fcntl.h>
#include <errno.h>

/* --------- Function Prototypes --------- */
void *server_listen(void *arg);
void *server_respond(void *arg);
void error(const char *msg);
/* ------- End Function Prototypes ------- */

server_data_t svr_data;

int main(int argc, char *argv[])
{
  bool found_new_idx;
  int flags;

  /* Zero-out all server data */
  memset(&svr_data, 0, sizeof(svr_data));

  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  svr_data.sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (svr_data.sockFd < 0)
  {
    error("ERROR opening socket");
  }

  memset((char *)&svr_data.servAddr, 0, sizeof(svr_data.servAddr));
  svr_data.portNo = atoi(argv[1]);

  svr_data.servAddr.sin_family = AF_INET;
  svr_data.servAddr.sin_addr.s_addr = INADDR_ANY;
  svr_data.servAddr.sin_port = htons(svr_data.portNo);

  if (bind(svr_data.sockFd, (struct sockaddr *)&svr_data.servAddr, sizeof(svr_data.servAddr)) < 0)
  {
    error("Binding Failed.");
  }
  listen(svr_data.sockFd, 5);
  svr_data.clntLen = sizeof(svr_data.clntAddr);

  while(1)
  {
    if(0 <= svr_data.currClntIdx) /* Only poll for new connections if there is enough free space(s) */
    {
      /* Poll to see if any clients are willing to connect */
      svr_data.svrConn[svr_data.currClntIdx].pfd.fd = svr_data.sockFd;
      svr_data.svrConn[svr_data.currClntIdx].pfd.events = POLLIN;
      if( 0 < poll(&svr_data.svrConn[svr_data.currClntIdx].pfd, 1, 1000/* Wait 1 second */) )
      {
        if( POLLIN & svr_data.svrConn[svr_data.currClntIdx].pfd.revents )
        {
          /* connect and create thread etc... */
          svr_data.svrConn[svr_data.currClntIdx].clntSockFd = accept(svr_data.sockFd, (struct sockaddr *)&svr_data.clntAddr, &svr_data.clntLen);

          flags = fcntl(svr_data.svrConn[svr_data.currClntIdx].clntSockFd, F_GETFL, 0);
          fcntl(svr_data.svrConn[svr_data.currClntIdx].clntSockFd, F_SETFL, flags | O_NONBLOCK);

          if (svr_data.svrConn[svr_data.currClntIdx].clntSockFd < 0)
          {
            error("Error on Accept");
          }

          svr_data.svrConn[svr_data.currClntIdx].thr_state = RUNNING;
          pthread_create(&(svr_data.svrConn[svr_data.currClntIdx].thrListen),  NULL, server_listen,  &svr_data.svrConn[svr_data.currClntIdx]);
          pthread_create(&(svr_data.svrConn[svr_data.currClntIdx].thrRespond), NULL, server_respond, &svr_data.svrConn[svr_data.currClntIdx]);

          /* Find the next available client index */
          found_new_idx = false;
          for(int i=0;i<MAX_CLIENT_NUM;i++)
          {
            if(UNUSED == svr_data.svrConn[i].thr_state)
            {
              svr_data.currClntIdx = i;
              found_new_idx = true;
              break;
            }
          }
          if(false == found_new_idx)
          {
            svr_data.currClntIdx = -1;
            printf("No new indexes found!");
          }
        }else{
          printf("Unrelated result from polling? \n");
        }
      }
    }

    /* Find a thread that is complete. Join it and use it for the next client that wants to connect */
    for(int i=0;i<MAX_CLIENT_NUM;i++)
    {
      if(STOPPING == svr_data.svrConn[i].thr_state)
      {
        printf("Main thread closing client %d...\n", i);
        // Wait for the threads to finish
        pthread_join(svr_data.svrConn[i].thrListen, NULL);
        pthread_join(svr_data.svrConn[i].thrRespond, NULL);
        svr_data.svrConn[i].thr_state = JOINED;
        printf("Successfully closed client %d\n", i);

        close(svr_data.svrConn[i].clntSockFd);

        svr_data.svrConn[i].thr_state = UNUSED;
        svr_data.currClntIdx = i;
      }
    }
  }

  close(svr_data.sockFd);
  return 0;
}

void *server_listen(void *arg)
{
  server_conn_t *svr_dat = (server_conn_t *)arg;

  int n;
  while (1)
  {
    memset(svr_dat->rx_buff, 0, sizeof(svr_dat->rx_buff));
    n = read(svr_dat->clntSockFd, svr_dat->rx_buff, 255); /* Blocking wait */
    if (n > 0)
    {
      printf("Client: %s\n", svr_dat->rx_buff);
    }

    /* Stop condition for server */
    int l = strncmp("Bye", svr_dat->rx_buff, 3);
    if (0 == l)
    {
      printf("Stop command received to receiver thread\n");
      svr_dat->thr_state = STOPPING;
      break;
    }
  }

  return (void *)0;
}

void *server_respond(void *arg)
{
  server_conn_t *svr_dat = (server_conn_t *)arg;
  int n;

  // Set stdin to non-blocking
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  while (1)
  {
    /* Wait for user-input into the tx-buffer */
    if (fgets(svr_dat->tx_buff, sizeof(svr_dat->tx_buff), stdin) != NULL) {
      n = write(svr_dat->clntSockFd, svr_dat->tx_buff, strlen(svr_dat->tx_buff));
      if (n < 0)
      {
        error("ERROR on Writing.");
      }
    }

    /* Stop condition, set by `server_listen` */
    if (STOPPING == svr_dat->thr_state)
    {
      printf("Stop command received to respond thread\n");
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
