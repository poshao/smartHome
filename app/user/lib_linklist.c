#include "lib_linklist.h"

/**
 * 链表实现
 */

lib_linklist_t *initList(){
    lib_linklist_t *list;
    list=os_zalloc(sizeof(lib_linklist_t));
    return list;
}

void add(lib_linklist_t *list, lib_linklist_item_t *item){
    if (list->header==NULL)
    {
        list->header=item;
        list->footer=item;
    }else{
        item->next=NULL;
        list->footer->next=item;
        list->footer=item;
    }
}

lib_linklist_item_t *exist(lib_linklist_t *list,char *key){
    lib_linklist_item_t *item;
    item=list->header;
    while (item)
    {
        if(os_strcmp(item->key,key)==0){
            return item;
        }
        item=item->next;
    }
}



void freeList(lib_linklist_t *list){
    lib_linklist_item_t *item,*next;
    item=list->header;

    while (item)
    {
        next=item->next;

        os_free(item->key);
        os_free(item->val);
        os_free(item);

        item=next;
    }
    os_free(list);
    list=NULL;
}