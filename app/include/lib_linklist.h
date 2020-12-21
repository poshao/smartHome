#ifndef __LIB_LINKLIST_H__
#define __LIB_LINKLIST_H__
#include "osapi.h"
#include "mem.h"

typedef struct lib_linklist_item{
    // void *data;
    char *key;
    char *val;
    lib_linklist_item_t *next;

}lib_linklist_item_t;

typedef struct lib_linklist{
    lib_linklist_item_t *header;
    lib_linklist_item_t *footer;
}lib_linklist_t;

#endif