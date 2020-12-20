#ifndef __SM_HTTP_PARSE_H__
#define __SM_HTTP_PARSE_H__
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

typedef enum sm_http_method{
    SM_HTTP_GET,
    SM_HTTP_POST
}sm_http_method_t;

typedef struct sm_http_request
{
    sm_http_method_t method;
    char *url;
    char **headers;
    char *body;
    uint16 bodylen;
    uint16 state;

}sm_http_request_t;

typedef struct sm_buf
{
    char *pos;
    char *last;
}sm_buf_t;

typedef enum sm_return{
    SM_OK,
    SM_INVALID_REQUEST_METHOD
}sm_return_t;

sm_return_t sm_parse_http(sm_http_request_t *r, sm_buf_t *b);
#endif