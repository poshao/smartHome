#include "osapi.h"
#include "at_custom.h"
#include "mem.h"
#include "user_interface.h"
#include "user_webserver.h"
#include "spi_flash.h"
#include "sntp.h"
#include "light_control.h"

#define AP_CLOSE_TIME 20

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

static int apCloseTime=AP_CLOSE_TIME;
static os_timer_t timerApClose;

void onCloseAP(void *arg){
    os_printf("close ap left time: %ds\n",apCloseTime);
    if(wifi_softap_get_station_num()>0 || wifi_station_get_connect_status()!=STATION_GOT_IP){
        apCloseTime=AP_CLOSE_TIME;
    }else if(apCloseTime--<0){
        if(wifi_get_opmode() & SOFTAP_MODE){
            wifi_set_opmode_current(STATION_MODE);
            os_timer_disarm(&timerApClose);
            os_printf("close ap\n");
        }
    }
}

// 初始化完成后的回调
void ICACHE_FLASH_ATTR on_init_done(void)
{
    os_printf("init finished!\r\n");

    char temp[20];
    uint32 chipid;

    chipid=system_get_chip_id();
    // 配置热点信息
    if(STATIONAP_MODE!=wifi_get_opmode_default()){
        struct softap_config ap_config;
        struct station_config sta_config;

        // 初始化设定
        os_printf("reset config...\n");

        // 需在保存station配置之前配置成包含StationMode的模式
        wifi_set_opmode(STATIONAP_MODE);

        wifi_station_get_config_default(&sta_config);
        strcpy(sta_config.ssid,"Snoopy");
        strcpy(sta_config.password,"helloboy");
        sta_config.all_channel_scan=TRUE;
        os_printf("station save: %d\n",wifi_station_set_config(&sta_config));

        wifi_softap_get_config_default(&ap_config);
        os_sprintf(temp,"SWAP%02X",chipid);
        strcpy(ap_config.ssid,temp);
        strcpy(ap_config.password,"12345678");
        ap_config.authmode=AUTH_WPA2_PSK;
        ap_config.max_connection=4;
        wifi_softap_set_config(&ap_config);
    }

    wifi_station_connect();

    os_sprintf(temp,"SWSTA%02X",chipid);
    wifi_station_set_hostname(temp);

    // 20秒自动关闭AP
    apCloseTime=AP_CLOSE_TIME;
    os_timer_disarm(&timerApClose);
    os_timer_setfn(&timerApClose,onCloseAP,NULL);
    os_timer_arm(&timerApClose,1000,TRUE);

    wifi_set_event_handler_cb(on_wifi_event);

    // 初始化IO
    light_init();

    // 启动服务
    startServer();
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

    // 设置mac v10
    // char client_mac[6]={0xDC,0x72,0x9B,0xA3,0x84,0x3B};
    // wifi_set_macaddr(STATION_IF,client_mac);

    // init SNTP
    // sntp_setservername(0,"us.pool.ntp.org");
    // sntp_setservername(1,"ntp.sjtu.edu.cn"); 
    // sntp_init();

    system_init_done_cb(on_init_done);
}
