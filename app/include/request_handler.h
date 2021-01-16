#ifndef __REQUEST_HANDLER_H__
#define __REQUEST_HANDLER_H__

#include "sm_http_parse.h"

void on_request(void *arg,sm_http_request_t *req);

#endif