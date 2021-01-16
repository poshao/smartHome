#include "osapi.h"
#include "gpio.h"
#include "light_control.h"

void light_init(void){
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
}

bool light_status(){
    os_printf("light status: %d\n",GPIO_INPUT_GET(5));
    return GPIO_INPUT_GET(5) ^ LIGHT_OUT_FLIP;
}   

void light_on(void){
    os_printf("light on\n");
    GPIO_OUTPUT_SET(5,TRUE^LIGHT_OUT_FLIP);
}

void light_off(void){
    os_printf("light off\n");
    GPIO_OUTPUT_SET(5,FALSE^LIGHT_OUT_FLIP);
}

// 延时关闭
static int gLightOffDelaySeconds;
static os_timer_t tmLightOffDelay;

void onLightOffDelay(void *arg){
    // os_printf("light off delay: %ds\n",gLightOffDelaySeconds);
    if (--gLightOffDelaySeconds < 0){
        light_off();
        os_timer_disarm(&tmLightOffDelay);
    }
}

void light_off_delay(int seconds){
    gLightOffDelaySeconds=seconds;
    os_timer_disarm(&tmLightOffDelay);
    os_timer_setfn(&tmLightOffDelay,onLightOffDelay,NULL);
    os_timer_arm(&tmLightOffDelay,1000,TRUE);
}