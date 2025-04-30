extern "C"{
    #include <fcntl.h>
    #include <unistd.h>
    #include <zlib.h>
    #include "http_handler.hpp"
}  
#include "data_packer.hpp"
#include "server_cpp.hpp"
#include <iostream>
#include <thread>
#include <cerrno>
#include <chrono>

// g++ -Wall Externals/http_parser/http_parser.o server/http_handler.cpp server/server_cpp.cpp server/data_packer.cpp -IExternals/http_parser -Iserver -lz

/* --------- Function Prototypes --------- */
void error(const char *msg);
/* ------- End Function Prototypes ------- */

using namespace std;

int on_message_beginlol(http_parser *parser) {
    printf("Add code to on_message_begin\n");
    return 0;
}

int main(int argc, char *argv[]){  
    server_data svr_data;
    
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
    listen(svr_data.sockFd, MAX_CLIENT_NUM);
    svr_data.clntLen = sizeof(svr_data.clntAddr);
    svr_data.currIdx = 0;

    while(1)
    {   
        if(MAX_CLIENT_NUM > svr_data.currIdx){
            /* Create session on client connection */
            printf("Waiting for Client to connect...\n");
            Session *sesh = new Session( accept(svr_data.sockFd, (struct sockaddr *)&svr_data.clntAddr, &svr_data.clntLen) ); /* TODO: Heap allocation. Remember to clean later! */
            svr_data.seshList[svr_data.currIdx] = sesh;
            printf("Client connected\n");
            for(int i=0;i<MAX_CLIENT_NUM;i++){
                printf("svr_data.seshList[%d]: %X\n", i, (uint32_t*)svr_data.seshList[i]);
            }

            svr_data.currIdx++;
        }
        
    }

    printf("Terminating Program?\n");
    close(svr_data.sockFd);
    return 0;
}

int Session::on_message_begin(http_parser *parser) {
    printf("Add code to on_message_begin\n");
    return 0;
}

int Session::on_url(http_parser *parser, const char *at, size_t length) {
    printf("URL: %.*s\n", (int)length, at);
    return 0;
}

int Session::on_status(http_parser *parser, const char *at, size_t length) {
    printf("Add code to on_status\n");
    return 0;
}

int Session::on_header_field(http_parser *parser, const char *at, size_t length) {
    printf("Header field: %.*s\n", (int)length, at);
    return 0;
}

int Session::on_header_value(http_parser *parser, const char *at, size_t length) {
    printf("Header value: %.*s\n", (int)length, at);
    return 0;
}

int Session::on_headers_complete(http_parser *parser) {
    printf("Add code to on_headers_complete\n");
    return 0;
}

int Session::on_body(http_parser *parser, const char *at, size_t length) {
    printf("Body: %.*s\n", (int)length, at);
    return 0;
}

int Session::on_message_complete(http_parser *parser) {
    printf("Message complete!\n");
    return 0;
}

int Session::on_chunk_header(http_parser *parser){
    printf("Add code to on_chunk_header\n");
    return 0;
}

int Session::on_chunk_complete(http_parser *parser){
    printf("Add code to on_chunk_complete\n");
    return 0;
}

Session::Session(int sockfd){
    printf("Session Created\n");
    clntSockFd = sockfd;
    killSesh = false;

    http_parser_settings_init(&(this->settings));
    settings.on_message_begin = on_message_begin;
    settings.on_url = Session::on_url;
    settings.on_status = Session::on_status;
    settings.on_header_field = Session::on_header_field;
    settings.on_header_value = Session::on_header_value;
    settings.on_headers_complete = Session::on_headers_complete;
    settings.on_body = Session::on_body;
    settings.on_message_complete = Session::on_message_complete;
  
    settings.on_chunk_header = Session::on_chunk_header;
    settings.on_chunk_complete = Session::on_chunk_complete;
  
    http_parser_init(&parser, HTTP_REQUEST);

    seshThread = thread(&Session::run, this);
}

Session::~Session(){
    printf("Session Closed\n");
    if (seshThread.joinable()) {
        seshThread.join();
    }
    close(clntSockFd);
    killSesh = true;
}

void Session::run(){
    StdRet_t procRet;
    while(false == killSesh){
        
        listen();
        procRet = process();
        if(E_QUIT == procRet){
            break;
        }else{
            respond();
        }
    }
}

void Session::listen(){
    read(clntSockFd, rx_buff, RX_BUFF_SIZE); /* Blocking wait */
}

StdRet_t Session::process(){
    return E_OK;
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

void Session::respond(){
    int body_len=1024;
    gzip_compress(response_body_template, sizeof(response_body_template)-1, response_body, &body_len);
    snprintf(tx_buff, TX_BUFF_SIZE, "HTTP/1.1 200 OK\r\n"
                                              "Date: Thu, 25 Apr 2025 12:34:56 GMT\r\n"
                                              "Server: Apache/2.4.41 (Ubuntu)\r\n"
                                              "Content-Type: text/html; charset=UTF-8\r\n"
                                              "Content-Encoding: gzip\r\n"
                                              "Content-Length: %d\r\n"
                                              "Connection: keep-alive\r\n"
                                              "\r\n",
                                              body_len);
    int start_point = strlen(tx_buff);
    for(int i=start_point;i<start_point+body_len;i++)
    {
      tx_buff[i] = response_body[i-start_point];
    }
    int write_size = data_size(tx_buff, TX_BUFF_SIZE, 0xA5); /* Account for null-terminator */

    write(clntSockFd, tx_buff, write_size);
}

void error(const char *msg)
{
  perror(msg);
  exit(1);
}
