#include "server.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <zlib.h>

#include "data_packer.h"

/* --------- Function Prototypes --------- */
void *thr_svr(void *arg);
int server_listen(server_conn_t *svr_dat);
int server_respond(server_conn_t *svr_dat);
void error(const char *msg);
void printf_cl(char* str, int cli_num);

Std_Return getPage(char* file_path, char* file_content, int file_content_len);
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
          pthread_create(&(svr_data.svrConn[svr_data.currClntIdx].thrSvr), NULL, thr_svr, &svr_data.svrConn[svr_data.currClntIdx]);

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
        pthread_join(svr_data.svrConn[i].thrSvr, NULL);
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

Std_Return getPage(char* file_path, char* file_content, int file_content_len)
{
  Std_Return retVal = E_OK;
  FILE* file;
  int ch;
  int ch_count = 0;
  file = fopen(file_path, "r");
  if(NULL == file)
  {
    retVal = -1;
    error("");
  }

  while(true)
  {
    ch = fgetc(file);
    if( (EOF ==ch) || (file_content_len <= ch_count) )
    {
      break;
    }
    else
    {
      file_content[ch_count] = (char) ch;
    }
    ch_count++;
  }

  return retVal;
}

void *thr_svr(void *arg)
{
  server_conn_t *svr_dat = (server_conn_t *)arg;

  svr_dat->httpprsr.data = svr_dat;

  /* Set up HTTP parser settings and call-back functions */
  http_parser_settings_init(&svr_dat->httpprsr_settings);
  svr_dat->httpprsr_settings.on_url = on_url;

  http_parser_init(&svr_dat->httpprsr, HTTP_REQUEST);

  while(1){
    server_listen(svr_dat);
    if(E_OK != server_respond(svr_dat))
    {
      break;
    }
  }

  return (void*)0;
}

int server_listen(server_conn_t *svr_dat)
{
  int retVal = E_OK;
  int n;

  memset(svr_dat->rx_buff, 0, sizeof(svr_dat->rx_buff));
  n = read(svr_dat->clntSockFd, svr_dat->rx_buff, RX_BUFF_SIZE); /* Blocking wait */
  if ( (0 < n) && (RX_BUFF_SIZE >= n) )
  {
    http_parser_execute(&svr_dat->httpprsr, &svr_dat->httpprsr_settings, svr_dat->rx_buff, strlen(svr_dat->rx_buff));

    /* Get file location from URL */
    char tmpPath_str[1024*2];
    snprintf(tmpPath_str, sizeof(tmpPath_str)-1, "server/pages%s", svr_dat->url);
    printf("\nURL: %s\n", tmpPath_str);

    /* READ HTML FILE */
    getPage(tmpPath_str, svr_dat->response_body_template, FILE_SIZE_MAX);

    /* PUBLISH HTML FILE */
    memset(svr_dat->tx_buff, 0xA5, sizeof(svr_dat->tx_buff));
    int body_len=FILE_SIZE_MAX;
    gzip_compress(svr_dat->response_body_template, sizeof(svr_dat->response_body_template)-1, svr_dat->response_body, &body_len);
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
      svr_dat->tx_buff[i] = svr_dat->response_body[i-start_point];
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
  }

  return retVal;
}

int server_respond(server_conn_t *svr_dat)
{
  int retVal = E_OK;
  int n;

  // Set stdin to non-blocking
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

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
    retVal = E_NOT_OK;
  }

  return retVal;
}

void error(const char *msg)
{
  perror(msg);
  exit(1);
}
