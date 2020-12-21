#ifndef __SM_DEBUG_H__
#define __SM_DEBUG_H__

#include "osapi.h"
#include "user_interface.h"

/**
 * print system state
 */
void sm_dump_system_state(){
    os_printf(">>> system state <<<\n");
    os_printf(">>> Cpu: %ld MHz\n",system_get_cpu_freq());
    os_printf(">>> Mem: %ld bytes\n",system_get_free_heap_size());
    os_printf(">>> system state <<<\n\n");
}

#endif