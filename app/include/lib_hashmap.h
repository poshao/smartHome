#ifndef __LIB_HASHMAP_H__
#define __LIB_HASHMAP_H__
#include "osapi.h"

typedef struct{
	int key;  //键
	int val;  //值
}DataType; //对基本数据类型进行封装，类似泛型

typedef struct{
	DataType data;
	struct HashNode *next;  //key冲突时，通过next指针进行连接
}HashNode;

typedef struct{
	int size;
	HashNode *table;
}HashMap,*hashmap;

#endif