#ifndef __USER_WEBSERVER_H__
#define __USER_WEBSERVER_H__
enum http_method
{
    METHOD_GET = 1,
    METHOD_POST
};

struct http_request
{
    char *url;
    enum http_method method;
    char *content_type;
    char *body;
    uint16 body_length;
};


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

#endif
