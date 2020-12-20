#ifndef __USER_WEBSERVER_H__
#define __USER_WEBSERVER_H__
enum http_method
{
    METHOD_GET = 1,
    METHOD_POST
};

struct http_header{
    char *name;
    char *value;
};

typedef struct http_request
{
    enum http_method method;
    char *url;
    struct  http_header **headers;
    char *content_type;
    char *body;
    uint16 body_length;
}http_request_t;

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
