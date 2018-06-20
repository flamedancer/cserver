/* tools/utils.c
*/

#include <errno.h>        /* errno */
#include <stdio.h>       /* NULL */
#include <stdlib.h>    /* free malloc calloc */
#include "utils.h"


void initListItem(struct ListItem * listItem) {
    listItem->next=NULL;
    listItem->value=NULL;
}

void initList(struct List * list) {
    list->start=list->end=NULL;
    list->length=0;
}

/* 在list尾端添加item
1. 若list为空，首尾都指向item
2. 否则，尾端的下一项指向item, 再置尾端为item
3. length + 1
*/
void listAppend(struct List* list, struct ListItem* item) {
    item->next = NULL;
    if(list->start == NULL) {
        list->start = list->end = item;
    } else {
        list->end->next = item;
        list->end = item;
    }
    list->length++;
}

/* 在 index 左边插入元素
 若index越界，认为是append
*/
void listInsert(struct List* list, int index, struct ListItem* item) {
    if(index < 0) {
        index = 0;
    }
    if(index >= list->length) {
        listAppend(list, item);
        return;
    }
    item->next = NULL;
    struct ListItem* insertpoint = list->start;
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
    struct ListItem* point = list->start;
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
struct ListItem* listGet(struct List* list, int index) {
    if(index >= list->length || index < 0) {
        perror("Error: Index Out Of Range");
    }
    struct ListItem* point = list->start;
    for(int step = 0; step < index; step++) {
        point = point->next;
    }
    return point;
}

void listSet(struct List* list, int index, struct ListItem* item) {
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
        struct ListItem* point = list->start;
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

void listRemove(struct List* list, struct ListItem* item) {}

void initMap(struct Map* map){
    map->table_len = HashTableLen;
    map->item_cnt = 0;
    for(int i=0; i<map->table_len; i++) {
        map->table[i] = NULL;
    }
}

void realseMap(struct Map* map) {
    for(int i=0; i<map->table_len; i++) {
        if(map->table[i] != NULL) {
            free(map->table[i]);
        }
    }
}

int hashCode(char* str) {
    int code;
    for(code=0; *str != '\0'; str++) {
        code = 31 * code + *str;
    }
    return code % HashTableLen;
}

void MapPush(struct Map* map, struct ListItem* item) {
    int index = 0;
    
}



