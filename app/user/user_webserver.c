#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "json/jsonparse.h"
#include "json/jsontree.h"
#include "user_interface.h"

#include "user_webserver.h"

#include "sm_http_parse.h"
#include "sm_debug.h"

char* ICACHE_FLASH_ATTR stristr(const char* pString, const char* pFind)
{
    char* char1 = NULL;
    char* char2 = NULL;
    if((pString == NULL) || (pFind == NULL) || (os_strlen(pString) < os_strlen(pFind)))
    {
        return NULL;
    }
   
    for(char1 = (char*)pString; (*char1) != '\0'; ++char1)
    {
        char* char3 = char1;
        for(char2 = (char*)pFind; (*char2) != '\0' && (*char1) != '\0'; ++char2, ++char1)
        {
            char c1 = (*char1) & 0xDF;
            char c2 = (*char2) & 0xDF;
            if((c1 != c2) || (((c1 > 0x5A) || (c1 < 0x41)) && (*char1 != *char2))) // 此处重新编辑了下
                break;
        }
       
        if((*char2) == '\0')
            return char3;
       
        char1 = char3;
    }
    return NULL;
}
#define os_stristr stristr

struct espconn *pconn = NULL;

static uint32 dat_sumlength = 0;
// 检查数据完整
bool ICACHE_FLASH_ATTR check_data(char *precv, uint16 length)
{
    //bool flag = true;
    char length_buf[10] = {0};
    char *ptemp = NULL;
    char *pdata = NULL;
    char *tmp_precvbuffer;
    uint16 tmp_length = length;
    uint32 tmp_totallength = 0;

    ptemp = (char *)os_strstr(precv, "\r\n\r\n");

    if (ptemp != NULL)
    {
        tmp_length -= ptemp - precv;
        tmp_length -= 4;
        tmp_totallength += tmp_length;

        pdata = (char *)os_strstr(precv, "Content-Length: ");

        if (pdata != NULL)
        {
            pdata += 16;
            tmp_precvbuffer = (char *)os_strstr(pdata, "\r\n");

            if (tmp_precvbuffer != NULL)
            {
                os_memcpy(length_buf, pdata, tmp_precvbuffer - pdata);
                dat_sumlength = atoi(length_buf);
                os_printf("A_dat:%u,tot:%u,lenght:%u\n", dat_sumlength, tmp_totallength, tmp_length);
                if (dat_sumlength != tmp_totallength)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

// #define newRequest() os_zalloc(sizeof(struct http_request))
// #define freeRequest(s) os_free(s->url);os_free(s->content_type);os_free(s->body);os_free(s);s=NULL;

// struct http_request* ICACHE_FLASH_ATTR newRequest(){
//     return os_zalloc(sizeof(struct http_request));
// }
// void ICACHE_FLASH_ATTR freeRequest(struct http_request *req){
//     os_free(req->url);
//     os_free(req->mime);
//     os_free(req->body);
//     os_free(req);
// }

// void ICACHE_FLASH_ATTR sendError(char *msg){
//     os_printf(msg);
// }

// void ICACHE_FLASH_ATTR parseRequest(struct http_request *req,char *pdata,uint16 len){
//     // 1.解析协议及方法路径
//     // 2.解析头文件
//     // 3.解析Body

//     char tmp[500];
//     char *precv,*pnext,*pbody;
//     int ilen=0;

//     pnext=os_strstr(pdata,"\r\n\r\n");
//     if(!pnext){
//         sendError("header end not found");
//         return;
//     }
//     *pnext=0;
//     *(pnext+1)=0;
//     pbody=pnext+4;

//     pnext=os_strstr(pdata,"\r\n")
// }


// 解析请求报文
// void ICACHE_FLASH_ATTR decodeRequest(struct http_request *req, char *pdata, uint16 len)
// {
//     /*
//     POST / HTTP/1.1
//     Content-Type:application/json
//     Content-Length:22

//     {code:200,msg:'hello'}
//     */
//     char *precv = pdata;
//     char *ptmp = NULL;
//     char *ptmp2=NULL;

//     if (os_strncmp(precv, "GET", 3) == 0)
//     {
//         req->method = METHOD_GET;
//         precv += 4;
//     }
//     else if (os_strncmp(precv, "POST", 4) == 0)
//     {
//         req->method = METHOD_POST;
//         precv += 5;
//     }
//     else
//     {
//         os_printf("bad request\n");
//         return;
//     }

//     ptmp = os_strstr(precv, " HTTP");
//     if (ptmp == NULL)
//     {
//         os_printf("bad request\n");
//         return;
//     }

//     // path
//     req->url = os_zalloc(ptmp - precv + 1);
//     os_strncpy(req->url, precv, ptmp - precv);
//     precv = ptmp + 7;

//     if (req->method == METHOD_GET)
//         return;

//     // length
//     ptmp = os_stristr(precv, "Content-Length: ");
//     if (ptmp != NULL)
//     {
//         ptmp2 = ptmp + 16;
//         ptmp = os_strstr(ptmp2, "\r\n");
//         if (ptmp == NULL)
//         {
//             os_printf("bad request\n");
//             return;
//         }

//         char *plen = os_zalloc(ptmp - ptmp2 + 1);
//         os_memcpy(plen, ptmp2, ptmp - ptmp2);
//         req->body_length = atoi(plen);
//         os_free(plen);
//         plen = NULL;


//         ptmp=os_strstr(precv,"Content-Type: ");
//         if(ptmp!=NULL){
//             ptmp+=14;
//             ptmp2=os_strstr(ptmp,"\r\n");
//             req->content_type=os_zalloc(ptmp2-ptmp+1);
//             os_memcpy(req->content_type,ptmp,ptmp2-ptmp);
//         }

//         ptmp = os_strstr(ptmp, "\r\n\r\n");
//         if (ptmp == NULL)
//         {
//             os_printf("bad request\n");
//         }

//         req->body = (char *)os_zalloc(req->body_length + 1);
//         os_memcpy(req->body, ptmp + 4, req->body_length);
//     }
// }

#define newResponse() os_zalloc(sizeof(struct http_response))
#define freeResponse(s) os_free(s->code);os_free(s->content_type);os_free(s->content);os_free(s);s=NULL;

// struct http_response* newResponse(){
//     return os_zalloc(sizeof(struct http_response));
// }
// void freeResponse(struct http_response *response){
//     os_free(response->code);
//     os_free(response->content_type);
//     os_free(response->content);
//     os_free(response);
// }

// char* ICACHE_FLASH_ATTR getCode(int code){
//     switch (code)
//     {
//     case 200:
//         return "200 OK";
//     case 400:
//         return "400 Invalid Request";
//     default:
//         return "500 Internal Server Error";
//     }
// }

// // 生成响应报文
// char* ICACHE_FLASH_ATTR encodeResponse(int code,char *data,int len,int *rawlen){
//     /*
//     HTTP/1.1 200 OK
//     Content-Type: application/json
//     Content-Length: 15

//     {"msg":"hello"}
//     */

//     char *raw=NULL;
//     int raw_length=0;
//     raw=os_zalloc(2048);

//     raw_length=os_sprintf(raw,"HTTP/1.1 %s\r\n",getCode(code));
//     // raw_length+=os_sprintf(raw+raw_length,"Content-Type: application/json\r\n");
//     raw_length+=os_sprintf(raw+raw_length,"Content-Type: text/html\r\n");
//     raw_length+=os_sprintf(raw+raw_length,"Content-Length: %d\r\n\r\n",len);
//     os_memcpy(raw+raw_length,data,len);
//     raw_length+=len;
//     *rawlen=raw_length;
//     return raw;
// }


// int putch(int a){
//     os_printf("%c",a);
//     return  a;
// }
// void ICACHE_FLASH_ATTR jsonTest(){
     
//     struct jsontree_context json;
//     jsontree_reset(&json);
//     JSONTREE_OBJECT(root,JSONTREE_PAIR("msg","hello"));
//     jsontree_setup(&json,(struct jsontree_context*)&root,NULL);
//     json.putchar=putch;
//     // json.path=1;
//     while (jsontree_print_next(&json) && json.path <= json.depth);
// }

void ICACHE_FLASH_ATTR onConnected(void *arg)
{
    struct espconn *conn = arg;
    esp_tcp *p = conn->proto.tcp;
    os_printf("tcp connected\n");
    os_printf("ip: " IPSTR ":%d\n", IP2STR(p->remote_ip), p->remote_port);
}
void ICACHE_FLASH_ATTR onDisconnected(void *arg)
{
    os_printf("tcp disconnected\n");
}
void ICACHE_FLASH_ATTR onReconnected(void *arg, sint8 err)
{
    os_printf("tcp reconnected\n");
}

// #define RR "<!DOCTYPE html><html><head><title>Light Control</title></head><body><button onclick=\"window.location.href='%s'\">%s</button></body></html>"
#define RR "<!DOCTYPE html><html><head><title>Light Control</title></head><body><h1>Switch Control</h1><button style=\"font-size: 42px;width:100%;height:300px;\" onclick=\"window.location.href='%s'\">%s</button></body></html>"
void ICACHE_FLASH_ATTR onRecv(void *arg, char *pdata, unsigned short len)
{
    if (!check_data(pdata, len))
    {
        os_printf("invalid request");
    }

    os_printf("origin recv: %s\n\n", pdata);

    sm_http_request_t *r;
    sm_buf_t *buf;
    sm_return_t rs;

    buf=sm_init_buf(pdata,len);
    r=sm_init_http_request();
    rs=sm_parse_http_request(r,buf);
    
    os_printf("parse result: %d\n",(int)rs);
    sm_dump_http_request(r);
    
    os_printf("Host: [%s]\n",sm_http_header_get_value(&r->headers,"Host"));
    sm_free_http_request(r);
    sm_free_buf(buf);
    
    sm_dump_system_state();

    // response
    sm_http_response_t *rsp;
    char aa[2000]={0};
    buf=sm_init_buf(aa,2000);
    rsp=sm_init_http_response();
    rsp->body=os_zalloc(20);
    os_strcpy(rsp->body,"hello world!");
    rsp->bodylen=os_strlen(rsp->body);
    
    sm_build_http_response(rsp,buf);
    espconn_send(arg,aa,buf->last-buf->pos);
    os_printf("build: %s\n\n",buf->pos);
    sm_free_buf(buf);

    sm_free_http_response(rsp);
    sm_dump_system_state();

    // jsonTest();
    // espconn_send(arg, HTTP_RESPONSE, strlen(HTTP_RESPONSE));
}
void ICACHE_FLASH_ATTR onSent(void *arg)
{
    os_printf("send ok\n");
}

void ICACHE_FLASH_ATTR startServer(void)
{
    esp_tcp *ptcp = NULL;
    if (pconn != NULL)
    {
        stopServer();
    }

    pconn = (struct espconn *)os_zalloc(sizeof(struct espconn));
    ptcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));

    ptcp->local_port = 80;
    ptcp->connect_callback = onConnected;
    ptcp->disconnect_callback = onDisconnected;
    ptcp->reconnect_callback = onReconnected;

    pconn->type = ESPCONN_TCP;
    pconn->state = ESPCONN_NONE;
    pconn->proto.tcp = ptcp;
    pconn->recv_callback = onRecv;
    pconn->sent_callback = onSent;

    espconn_accept(pconn);

    // 创建关闭连接的任务

    os_printf("start server :80 ... OK!\n");
}

void ICACHE_FLASH_ATTR stopServer(void)
{
    espconn_delete(pconn);
    os_free(pconn->proto.tcp);
    os_free(pconn);
    pconn = NULL;
}