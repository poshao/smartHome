#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// #define AT_CUSTOM_UPGRADE

#ifdef AT_CUSTOM_UPGRADE
    #ifndef AT_UPGRADE_SUPPORT
    #error "upgrade is not supported when eagle.flash.bin+eagle.irom0text.bin!!!"
    #endif
#endif

// #define CONFIG_AT_SMARTCONFIG_COMMAND_ENABLE
// #define CONFIG_AT_WPA2_ENTERPRISE_COMMAND_ENABLE

// #define CONFIG_ENABLE_IRAM_MEMORY       1
#endif
