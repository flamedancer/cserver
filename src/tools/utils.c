/* tools/utils.c
*/

#include <errno.h>        /* errno */
#include <stdio.h>       /* NULL */
#include <stdlib.h>    /* free malloc calloc */
#include <string.h>
#include "utils.h"


void initItem(struct Item * listItem) {
    listItem->next=NULL;
    listItem->key=NULL;
    listItem->value=NULL;
}

void initList(struct List * list) {
    list->start=list->end=NULL;
    list->length=0;
}

struct Item * newItem(char * key, void * value) {
    struct Item * item = (struct Item *) malloc(sizeof(struct Item));
    initItem(item);
    item->key = key;
    item->value = value;
    return item;
}

/* 在list尾端添加item
1. 若list为空，首尾都指向item
2. 否则，尾端的下一项指向item, 再置尾端为item
3. length + 1
*/
void listAppend(struct List* list, struct Item* item) {
    item->next = NULL;
    if(list->start == NULL) {
         list->start = list->end = item;
    }
     else {
         list->end->next = item;
         list->end = item;
     }
     list->length++;
}

/* 在 index 左边插入元素
 若index越界，认为是append
*/
void listInsert(struct List* list, int index, struct Item* item) {
    if(index < 0) {
        index = 0;
    }
    if(index >= list->length) {
        listAppend(list, item);
        return;
    }
    item->next = NULL;
    struct Item* insertpoint = list->start;
    for(int step = 0; step < index; step++) {
        insertpoint = insertpoint->next;
    }
    if (index == 0) {
        item->next = list->start;
        list->start = item;
    } else {
        item->next = insertpoint->next;
        insertpoint->next = item;
    }
    list->length++;
}


void listPrint(struct List* list) {
    struct Item* point = list->start;
    printf("[");
    for(int i=0; i<list->length; i++) {
        if( i>0 ) {
            printf(", ");
        }
        printf("'%s'", point->value);
        point = point->next;
    }
    printf("]\n");
}


/* 取 index 处元素
*/
struct Item* listGet(struct List* list, int index) {
    if(index >= list->length || index < 0) {
        perror("Error: Index Out Of Range");
    }
    struct Item* point = list->start;
    for(int step = 0; step < index; step++) {
        point = point->next;
    }
    return point;
}

void listSet(struct List* list, int index, struct Item* item) {
    if(index >= list->length || index < 0) {
        perror("Error: Index Out Of Range");
    }
    if(index == 0) {
        if( list->length == 1) {
            item->next = NULL;
            list->start = list->end = item;
        } else {
            item->next = list->start->next;
            list->start = item;
        }
    } else {
        struct Item* point = list->start;
        for(int step = 0; step < index; step++) {
            point = point->next;
        }
        if (index == list->length - 1){
            item->next = NULL;
            list->end = item;
        } else {
            item->next = point->next; 
        }
        point->next = item;
        
    }
}

void listRemove(struct List* list, struct Item* item) {}

void initMap(struct Map* map){
    map->table_len = HashTableLen;
    map->item_cnt = 0;
    for(int i=0; i<map->table_len; i++) {
        map->table[i] = NULL;
    }
}

void releaseMap(struct Map* map) {
    for(int i=0; i<map->table_len; i++) {
        if(map->table[i] != NULL) {
            /* free map 的同时 把所有 item free掉 */
            struct Item * point = map->table[i]->start;
            struct Item * nextPoint = NULL;
            for (int j = 0; j < map->table[i]->length; j++) {
                nextPoint = point->next;
                free(point);
                point = nextPoint;
            }
            free(map->table[i]);
            map->table[i] = NULL;
        }
    }
}

int hashCode(char * str) {
    int code;
    int len = 0;
    int maxLen = 100;
    for(code=0; *str != '\0' && len < maxLen; str++) {
        code = code + 31 * (*str);
        len++;
    }
    return code % HashTableLen;
}

void mapPush(struct Map* map, struct Item* item) {
    int index = hashCode(item->key);
    if(map->table[index] == NULL) {
         struct List* list = malloc(sizeof(struct List));
         initList(list);
         if(list == NULL) {
             perror("Error: out of storeage");
         }
         map->table[index] = list;
    } else {
        // 检查是否已经有key, 有则覆盖
        struct Item* _item;
        char * key = item->key;
        _item = map->table[index]->start;
        while(_item != NULL) {
            if(strcmp(key, _item->key) == 0 ) {
                _item->value = item->value;
                return;
            }
            _item = _item->next;
        }
    }
    listAppend(map->table[index], item);
    map->item_cnt++;
}

void mapPrint(struct Map* map) {
    struct List* list; 
    struct Item* item;
    int print_item_cnt = 0; 
    printf("{");
    for(int i=0; i<map->table_len; i++) {
         list = map->table[i];
         if(list == NULL) {
            continue;
         }
         item = list->start;
         while(item != NULL) {
             printf("'%s': '%s'", item->key, item->value);
             item = item->next;
             print_item_cnt++;
             if(print_item_cnt != map->item_cnt) {
                 printf(", ");
             }
         }
    }
    printf("}\n");
} 

/* 查找是否有这个 key 有则返回对应value 没有则返回null */
char * mapGet(struct Map * map, char * key) {
    int index = hashCode(key);
    if(map->table[index] == NULL) {
        return NULL;
    } else {
        struct Item* item;
        item = map->table[index]->start;
        while(item != NULL) {
            if(strcmp(key, item->key) == 0 ) {
                return item->value;
            }
            item = item->next;
        }
    }
    return NULL;
}
