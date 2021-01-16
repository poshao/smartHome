#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
// #include "json/jsonparse.h"
// #include "json/jsontree.h"
#include "user_interface.h"
// #include "spi_flash.h"
// #include "sntp.h"

#include "user_webserver.h"

#include "sm_http_parse.h"
#include "sm_debug.h"

#include "request_handler.h"

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

void send_error(void *arg,int code,char *msg){
    // response
    sm_http_response_t *rsp;
    sm_buf_t *buf;
    char aa[2000]={0};
    buf=sm_init_buf(aa,2000);
    rsp=sm_init_http_response();
    rsp->code=code;
    // sm_http_header_set(&rsp->headers,"Content-Type","text/html");
    sm_http_content_type_set(rsp,SM_CONTENT_TYPE_HTML);
    if(msg){
        rsp->bodylen=os_strlen(msg);
        rsp->body=os_zalloc(rsp->bodylen+1);
        os_memcpy(rsp->body,msg,rsp->bodylen);
    }
    sm_build_http_response(rsp,buf);
    espconn_send(arg,aa,buf->last-buf->pos);
    // os_printf("build: %s\n\n",buf->pos);
    sm_free_buf(buf);
    sm_free_http_response(rsp);
    // sm_dump_system_state();
}

/**
 * /index.html 未验证login.html 否则main.html
 * /main.css 样式表
 * /main.js 脚本
 * 
 * /wifi?info 获取wifi相关信息
 * /io?info 获取io相关信息
 * /sntp?info 获取sntp相关信息
 * /usart?info 获取usart相关信息
 * /help 打印帮助信息
 * /version 打印版本信息
 */ 

// void on_request(void *arg,sm_http_request_t *req){
//     if(req->method==SM_HTTP_GET){
//         if(os_strcmp(req->url,"/index.html")==0){
//             send_error(arg,200,"okay");
//         }else if(os_strcmp(req->url,"/pure-min.css")==0){
            
//         }else if(os_strcmp(req->url,"/light?on")==0){
//             light_on();
//             light_off_delay(3);
//             send_error(arg,200,"ok");
//         }else if(os_strcmp(req->url,"/light?off")==0){
//             light_off();
//             send_error(arg,200,"ok");
//         }else if(os_strcmp(req->url,"/light")==0){
//             light_status();
//             send_error(arg,200,"ok");

//         }else if(os_strcmp(req->url,"/now")==0){
//             send_error(arg,200,sntp_get_real_time(sntp_get_current_timestamp()));
//         }else{
//             send_error(arg,404,NULL);
//         }
//     }
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

void ICACHE_FLASH_ATTR onRecv(void *arg, char *pdata, unsigned short len)
{
    // uint32 current_timestamp;
    // current_timestamp=sntp_get_current_timestamp();
    // os_printf("time: %ld %s\n",current_timestamp,sntp_get_real_time(current_timestamp));

    if (!check_data(pdata, len))
    {
        os_printf("invalid request");
    }

    // os_printf("origin recv: %s\n\n", pdata);

    sm_http_request_t *r;
    sm_buf_t *buf;
    sm_return_t rs;

    buf=sm_init_buf(pdata,len);
    r=sm_init_http_request();
    rs=sm_parse_http_request(r,buf);
    sm_free_buf(buf);
    
    // sm_dump_http_request(r);
    
    if(rs==SM_OK){
        on_request(arg,r);
    }else{
        os_printf("parse error: %d\n",(int)rs);
    }
    // os_printf("parse result: %d\n",(int)rs);
    // sm_dump_http_request(r);
    
    sm_free_http_request(r);
    sm_dump_system_state();
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