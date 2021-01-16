#ifndef __USER_WEBSERVER_H__
#define __USER_WEBSERVER_H__
#include "os_type.h"

struct  http_response
{
    char proto[8];
    char *code;
    char *content_type;
    uint16 content_length;
    char *content;
};

void startServer(void);
void stopServer(void);

void send_error(void *arg,int code,char *msg);

#endif
