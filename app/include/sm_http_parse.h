#ifndef __SM_HTTP_PARSE_H__
#define __SM_HTTP_PARSE_H__
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

typedef struct sm_http_header
{
    char *name;
    char *value;

    struct sm_http_header *next;
} sm_http_header_t;

typedef struct sm_http_headers
{
    sm_http_header_t *header;
    sm_http_header_t *footer;
} sm_http_headers_t;

typedef enum sm_http_method
{
    SM_HTTP_INVALID = 0,
    SM_HTTP_GET,
    SM_HTTP_POST
} sm_http_method_t;

typedef struct sm_http_request
{
    sm_http_method_t method;
    char *url;
    sm_http_headers_t headers;
    char *body;
    uint16 bodylen;

    uint16 state;

} sm_http_request_t;

typedef struct sm_buf
{
    char *pos;
    char *last;
} sm_buf_t;

typedef enum sm_return
{
    SM_OK,
    SM_INVALID_REQUEST_METHOD,
    SM_NO_MEM
} sm_return_t;

// functions
sm_http_request_t *sm_init_http_request();
void sm_free_http_request(sm_http_request_t *r);
sm_buf_t *sm_init_buf(char *pdata, int len);
void sm_free_buf(sm_buf_t *buf);
sm_return_t sm_parse_http_request(sm_http_request_t *r, sm_buf_t *b);

void sm_http_header_set(sm_http_headers_t *headers, char *name, char *value);
sm_http_header_t *sm_http_header_get(sm_http_headers_t *headers, char *name);
char *sm_http_header_get_value(sm_http_headers_t *headers, char *name);

void sm_dump_http_request(sm_http_request_t *r);

typedef enum sm_content_type{
    SM_CONTENT_TYPE_HTML,
    SM_CONTENT_TYPE_JSON,
    SM_CONTENT_TYPE_X_FORM,
    SM_CONTENT_TYPE_JAVASCRIPT,
    SM_CONTENT_TYPE_CSS
}sm_content_type_t;

typedef struct sm_http_response
{
    int code;
    sm_http_headers_t headers;
    char *body;
    int bodylen;
}sm_http_response_t;

void sm_http_content_type_set(sm_http_response_t *r,sm_content_type_t tp);
sm_http_response_t *sm_init_http_response();
void sm_free_http_response(sm_http_response_t *r);
sm_return_t sm_build_http_response(sm_http_response_t *r,sm_buf_t *buf);
#endif