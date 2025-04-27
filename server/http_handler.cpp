#include "http_handler.hpp"
#include <iostream>
#include <cstring>

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
