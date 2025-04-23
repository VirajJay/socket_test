/* HTTP parse test */
#include <stdio.h>
#include <string.h>
#include "http_parser.h"

/* --------- How to compile ---------
gcc Externals/http_parser/http_parser.o Externals/http_parser/examples/http_parser_example.c -IExternals/http_parser/
./a.out
*/

int on_message_begin(http_parser *parser) {
    printf("Add code to on_message_begin\n");
    return 0;
}

int on_url(http_parser *parser, const char *at, size_t length) {
    printf("URL: %.*s\n", (int)length, at);
    return 0;
}

int on_status(http_parser *parser, const char *at, size_t length) {
    printf("Add code to on_status\n");
    return 0;
}

int on_header_field(http_parser *parser, const char *at, size_t length) {
    printf("Header field: %.*s\n", (int)length, at);
    return 0;
}

int on_header_value(http_parser *parser, const char *at, size_t length) {
    printf("Header value: %.*s\n", (int)length, at);
    return 0;
}

int on_headers_complete(http_parser *parser) {
    printf("Add code to on_headers_complete\n");
    return 0;
}

int on_body(http_parser *parser, const char *at, size_t length) {
    printf("Body: %.*s\n", (int)length, at);
    return 0;
}

int on_message_complete(http_parser *parser) {
    printf("Message complete!\n");
    return 0;
}

int on_chunk_header(http_parser *parser){
    printf("Add code to on_chunk_header\n");
    return 0;
}

int on_chunk_complete(http_parser *parser){
    printf("Add code to on_chunk_complete\n");
    return 0;
}

int main(int argc, char *argv[])
{
    char req_buff[500] ="GET /index.html HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: MyClient/1.0\r\n"
    "Accept: text/html\r\n"
    "Connection: keep-alive\r\n"
    "\r\n";

    char req_buff_body[500] = "POST /login HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 29\r\n"
    "\r\n"
    "username=admin&password=1234\r\n";

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

    size_t parsed = http_parser_execute(&parser, &settings, req_buff_body, strlen(req_buff_body));

    // if (parsed != strlen(req_buff)) {
    //     fprintf(stderr, "Parsing failed: %s (%s)\n",
    //             http_errno_name(HTTP_PARSER_ERRNO(&parser)),
    //             http_errno_description(HTTP_PARSER_ERRNO(&parser)));
    //     return 1;
    // }

    return 0;
}

