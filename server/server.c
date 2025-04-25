#include "server.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <zlib.h>

#include "http_handler.h"
#include "data_packer.h"

/* --------- Function Prototypes --------- */
void *server_listen(void *arg);
void *server_respond(void *arg);
void error(const char *msg);
void printf_cl(char* str, int cli_num);
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
          memset(&(svr_data.svrConn[svr_data.currClntIdx]), 0, sizeof(svr_data.svrConn[svr_data.currClntIdx]));

          /* connect and create thread etc... */
          svr_data.svrConn[svr_data.currClntIdx].clntSockFd = accept(svr_data.sockFd, (struct sockaddr *)&svr_data.clntAddr, &svr_data.clntLen);

          if (svr_data.svrConn[svr_data.currClntIdx].clntSockFd < 0)
          {
            error("Error on Accept");
          }
          flags = fcntl(svr_data.svrConn[svr_data.currClntIdx].clntSockFd, F_GETFL, 0);
          fcntl(svr_data.svrConn[svr_data.currClntIdx].clntSockFd, F_SETFL, flags | O_NONBLOCK);

          printf("Server Threads created. svr_data.currClntIdx - %d\n", svr_data.currClntIdx);
          svr_data.svrConn[svr_data.currClntIdx].thr_state = RUNNING;
          svr_data.svrConn[svr_data.currClntIdx].my_idx = svr_data.currClntIdx;
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

const char response_body_template[] = "<!DOCTYPE html>\r\n"
"<html lang='en'>\r\n"
"<head>\r\n"
"    <meta charset='UTF-8'>\r\n"
"    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n"
"    <title>Welcome to My Server</title>\r\n"
"</head>\r\n"
"<body>\r\n"
"    <h1>Whatup Dog!</h1>\r\n"
"    <p>Get request worked yo!.</p>\r\n"
"    <p>WOOOOOOOOOOOOOOOOOOOOO!</p>\r\n"
"</body>\r\n"
"</html>\r\n";

unsigned char response_body[1024];

void *server_listen(void *arg)
{
  server_conn_t *svr_dat = (server_conn_t *)arg;
  int n;

  /* Set up HTTP parser settings and call-back functions */
  http_parser parser;
  http_parser_settings settings;

  http_parser_settings_init(&settings);
  settings.on_message_begin = on_message_begin;
  settings.on_url = on_url;
  settings.on_status = on_status;
  settings.on_header_field = on_header_field;
  settings.on_header_value = on_header_value;
  settings.on_headers_complete = on_headers_complete;
  settings.on_body = on_body;
  settings.on_message_complete = on_message_complete;

  settings.on_chunk_header = on_chunk_header;
  settings.on_chunk_complete = on_chunk_complete;

  http_parser_init(&parser, HTTP_REQUEST);

  while (1)
  {
    memset(svr_dat->rx_buff, 0, sizeof(svr_dat->rx_buff));
    n = read(svr_dat->clntSockFd, svr_dat->rx_buff, RX_BUFF_SIZE); /* Blocking wait */
    if ( (0 < n) && (RX_BUFF_SIZE >= n) )
    {
      printf("cli_num: %d  Start\n", svr_dat->my_idx);
      printf("cli_num: %d  Client: %s1", svr_dat->my_idx, svr_dat->rx_buff);
      printf("cli_num: %d  End\n", svr_dat->my_idx);
      http_parser_execute(&parser, &settings, svr_dat->rx_buff, strlen(svr_dat->rx_buff));

      memset(svr_dat->tx_buff, 0xA5, sizeof(svr_dat->tx_buff));
      int body_len=1024;
      gzip_compress(response_body_template, sizeof(response_body_template)-1, response_body, &body_len);
      snprintf(svr_dat->tx_buff, TX_BUFF_SIZE, "HTTP/1.1 200 OK\r\n"
                                                "Date: Thu, 25 Apr 2025 12:34:56 GMT\r\n"
                                                "Server: Apache/2.4.41 (Ubuntu)\r\n"
                                                "Content-Type: text/html; charset=UTF-8\r\n"
                                                "Content-Encoding: gzip\r\n"
                                                "Content-Length: %d\r\n"
                                                "Connection: keep-alive\r\n"
                                                "\r\n",
                                                body_len);
      int start_point = strlen(svr_dat->tx_buff);
      for(int i=start_point;i<start_point+body_len;i++)
      {
        svr_dat->tx_buff[i] = response_body[i-start_point];
      }
      int write_size = data_size(svr_dat->tx_buff, TX_BUFF_SIZE, 0xA5); /* Account for null-terminator */
      
      write(svr_dat->clntSockFd, svr_dat->tx_buff, write_size);
    }
    else if(RX_BUFF_SIZE < n)
    {
      printf("cli_num: %d  Reception Buffer Overflow\n", svr_dat->my_idx);
    }

    /* Stop condition for server */
    int l = strncmp("Bye", svr_dat->rx_buff, 3);
    if (0 == l)
    {
      printf("cli_num: %d  Stop command received to receiver thread\n", svr_dat->my_idx);
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
      printf("cli_num: %d  Stop command received to respond thread\n", svr_dat->my_idx);
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
