/**
 * 处理Web请求
 */

#include "osapi.h"
#include "sntp.h"
#include "stdlib.h"

#include "user_webserver.h"
#include "sm_http_parse.h"

#include "light_control.h"

#include "default_page.h"

void send_light_default_page(void *arg){
    char data[2000]={0};
    os_sprintf(data,DefaultPage,light_status()?"ON":"OFF");
    send_error(arg,200,data);
}

void on_request(void *arg,sm_http_request_t *req){
    if(req->method==SM_HTTP_GET){
        if(os_strstr(req->url,"/light?on")!=NULL){
            light_on();
            char *p;
            p=os_strstr(req->url,"on=");
            if(p){
                light_off_delay(atoi(p+3));
            }
            send_light_default_page(arg);
        }else if(os_strcmp(req->url,"/light?off")==0){
            light_off();
            send_light_default_page(arg);
        }else if(os_strcmp(req->url,"/reset")==0){
            system_restore();
            send_error(arg,200,"ok");
        // }else if(os_strcmp(req->url,"/now")==0){
        //     send_error(arg,200,sntp_get_real_time(sntp_get_current_timestamp()));
        }else{
            send_light_default_page(arg);
            // send_error(arg,404,NULL);
        }
        
    }else if(req->method ==SM_HTTP_POST){

    }
}

// void load_file(int index){
//     char a[100]={0};
//     spi_flash_read(0x200000,&a,100);
//     os_printf("read flash 0x200000: %s \n",a);
// }