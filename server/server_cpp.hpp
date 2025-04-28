#ifndef SERVER_CPP_HPP_
#define SERVER_CPP_HPP_

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
    E_OK = 0U,
    E_NOT_OK,
    E_QUIT
}StdRet_t;

class Session{
public:
    int clntSockFd;
    bool killSesh;
    // struct pollfd pfd;
    char tx_buff[TX_BUFF_SIZE];
    char rx_buff[RX_BUFF_SIZE];

    Session(int sockfd);
    ~Session();

    void run();

private:
    std::thread seshThread;
    void listen();
    StdRet_t process();
    void respond();
};

typedef struct server_data
{
  /* Client instances */
  Session *seshList[MAX_CLIENT_NUM];

  /* Main Server */
  int sockFd;    /* Main socket */
  int portNo;
  int currIdx;
  struct sockaddr_in servAddr;
  struct sockaddr_in clntAddr;
  socklen_t clntLen;
} server_data_t;

#endif /* SERVER_CPP_HPP_ */
