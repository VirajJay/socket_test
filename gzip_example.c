#include "data_packer.h"
#include <string.h>
#include <stdio.h>

char response_header[] = "HTTP/1.1 200 OK\r\n"
"Date: Thu, 25 Apr 2025 12:34:56 GMT\r\n"
"Server: Apache/2.4.41 (Ubuntu)\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Content-Encoding: gzip\r\n"
"Content-Length: 339\r\n"
"Connection: keep-alive\r\n"
"\r\n";

char response_body[] = "<!DOCTYPE html>\r\n"
"<html lang='en'>\r\n"
"<head>\r\n"
"    <meta charset='UTF-8'>\r\n"
"    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n"
"    <title>Welcome to My Server</title>\r\n"
"</head>\r\n"
"<body>\r\n"
"    <h1>Welcome to My Server!</h1>\r\n"
"    <p>This is a response to your GET request.</p>\r\n"
"    <p>Enjoy browsing!</p>\r\n"
"</body>\r\n"
"</html>\r\n";

char response_body_cmp[1024];

// gcc -Iserver/ server/data_packer.c gzip_example.c -lz

int main(int argc, char *argv[])
{
    
    int cmp_len;
    gzip_compress(response_body, sizeof(response_body), response_body_cmp, &cmp_len);

    printf("Original Len: %ld\n", sizeof(response_body));
    printf("cmp_len: %d\n", cmp_len);

    /* Save contents to a file */
    FILE *fp;
    fp = fopen("cmp_out.gzip", "w");
    if(NULL == fp){
        return -1;
    }
    fwrite(response_body_cmp, sizeof(char), sizeof(response_body_cmp)/sizeof(char), fp);
    fclose(fp);

    return 0;
}
