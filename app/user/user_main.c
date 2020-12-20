#include "osapi.h"
#include "at_custom.h"
#include "mem.h"
#include "user_interface.h"
#include "user_webserver.h"

#if (SPI_FLASH_SIZE_MAP == 0)
    #define SYSTEM_PARTITION_RF_CAL_ADDR 0x7b000
    #define SYSTEM_PARTITION_PHY_DATA_ADDR 0x7c000
    #define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x7d000

    #define IROM0TEXT_BIN_LEN 0x5c000

#elif (SPI_FLASH_SIZE_MAP == 2)
    #define SYSTEM_PARTITION_RF_CAL_ADDR 0xfb000
    #define SYSTEM_PARTITION_PHY_DATA_ADDR 0xfc000
    #define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0xfd000

    #define IROM0TEXT_BIN_LEN 0xbc000

#elif (SPI_FLASH_SIZE_MAP == 6)
    #define SYSTEM_PARTITION_RF_CAL_ADDR 0x3fb000
    #define SYSTEM_PARTITION_PHY_DATA_ADDR 0x3fc000
    #define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x3fd000

    #define IROM0TEXT_BIN_LEN 0xc0000
#endif

#define CheckError(s) \
    if (!s)           \
        os_printf("do error!\r\n");

#define EAGLE_FLASH_BIN_ADDR (SYSTEM_PARTITION_CUSTOMER_BEGIN + 1)
#define EAGLE_IROM0TEXT_BIN_ADDR (SYSTEM_PARTITION_CUSTOMER_BEGIN + 2)

static const partition_item_t partition_table[] = {
    // {EAGLE_FLASH_BIN_ADDR, 0x00000, 0x10000},
    // {EAGLE_IROM0TEXT_BIN_ADDR, 0x10000, IROM0TEXT_BIN_LEN},
    {SYSTEM_PARTITION_RF_CAL, SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000},
    {SYSTEM_PARTITION_PHY_DATA, SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000},
    {SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000},
};

// 禁用DIO-To-QIO
void ICACHE_FLASH_ATTR user_spi_flash_dio_to_qio_pre_init(void)
{
}

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    // 注册分区结构
    if (!system_partition_table_regist(partition_table, sizeof(partition_table)/sizeof(partition_table[0]), SPI_FLASH_SIZE_MAP))
    {
        os_printf("system_partition_table_regist fail\r\n");
    }
}

void on_wifi_event(System_Event_t *evt)
{
    // os_printf("event %x\n", evt->event);
    switch (evt->event)
    {
    case EVENT_STAMODE_CONNECTED:
        os_printf("[connected]connect to ssid %s, channel %d\n", evt->event_info.connected.ssid, evt->event_info.connected.channel);
        break;
    case EVENT_STAMODE_DISCONNECTED:
        os_printf("[disconnected]disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid, evt->event_info.disconnected.reason);
        break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
        os_printf("[auth mode change]mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
        break;
    case EVENT_STAMODE_GOT_IP:
        os_printf("[got ip]ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip), IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
        os_printf("\n");
        break;
    case EVENT_SOFTAPMODE_STACONNECTED:
        os_printf("[client connected]station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac), evt->event_info.sta_connected.aid);
        break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
        os_printf("[station disconnected]station: " MACSTR "leave, AID = %d\n", MAC2STR(evt->event_info.sta_disconnected.mac), evt->event_info.sta_disconnected.aid);
        break;
    case EVENT_SOFTAPMODE_PROBEREQRECVED:
        // os_printf("[probe request received]\n");
        break;
    default:
        break;
    }
}

// 设置热点
void ICACHE_FLASH_ATTR init_softap(void)
{
    struct softap_config *config = NULL;

    // if (wifi_get_opmode() != SOFTAP_MODE)
    // {
    //     CheckError(wifi_set_opmode(SOFTAP_MODE));
    // }

    config = (struct softap_config *)os_zalloc(sizeof(struct softap_config));
    strcpy(config->ssid, "Snoopy2");
    // os_memcpy(config->ssid,"Snoopy",7);
    config->ssid_len = strlen(config->ssid);
    strcpy(config->password, "12345678");
    config->channel = 11;
    config->max_connection = 4;
    config->ssid_hidden = 0;
    config->authmode = AUTH_WPA2_PSK;
    config->beacon_interval = 100;

    CheckError(wifi_softap_set_config_current(config));

    os_free(config);
    os_printf("init softAP(Snoopy:12345678)... OK!\r\n");

    wifi_set_event_handler_cb(on_wifi_event);
}

void ICACHE_FLASH_ATTR initClient(void){
    struct station_config *config=NULL;
    wifi_set_opmode(STATIONAP_MODE);
    config=(struct station_config*)os_zalloc(sizeof(struct station_config));
    strcpy(config->ssid,"Snoopy");
    strcpy(config->password,"helloboy");
    wifi_station_set_config_current(config);
    wifi_station_connect();

}

static int light_state=0;
void ICACHE_FLASH_ATTR onTimer(void *arg){
    os_printf("light:%d\n",light_state);
    GPIO_OUTPUT_SET(5,light_state);
    light_state=~light_state;
}

// flash light
void ICACHE_FLASH_ATTR initLight(){
    os_timer_t timer;

    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);


    os_timer_disarm(&timer);
    os_timer_setfn(&timer,onTimer,NULL);
    os_timer_arm(&timer,100,TRUE);
}

// 初始化完成后的回调
void ICACHE_FLASH_ATTR on_init_done(void)
{
    os_printf("init finished!\r\n");
    // PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    // PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);

    // initClient();
    init_softap();
    startServer();
    // initLight();
}

// 启动函数
void ICACHE_FLASH_ATTR user_init(void)
{
#ifdef DEBUG
    os_printf("chip id: 0x%x\r\n", system_get_chip_id());
    os_printf("compile time: "__DATE__
              " "__TIME__
              "\r\n");
    os_printf("sdk version: " ESP_SDK_VERSION_STRING "\r\n");
#else
    os_printf("compile time: "__DATE__
              " "__TIME__
              "\n");
#endif
    system_init_done_cb(on_init_done);
}
