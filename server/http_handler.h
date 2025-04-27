#ifndef HTTP_HANDLER_H_
#define HTTP_HANDLER_H_

#include "http_parser.h"

/* ---------- Function Prototypes ---------- */
int on_message_begin(http_parser *parser);
int on_url(http_parser *parser, const char *at, size_t length);
int on_status(http_parser *parser, const char *at, size_t length);
int on_header_field(http_parser *parser, const char *at, size_t length);
int on_header_value(http_parser *parser, const char *at, size_t length);
int on_headers_complete(http_parser *parser);
int on_body(http_parser *parser, const char *at, size_t length);
int on_message_complete(http_parser *parser);
int on_chunk_header(http_parser *parser);
int on_chunk_complete(http_parser *parser);

#endif /*HTTP_HANDLER_H_*/