/* test/utilsTest.c
    test cmd :
    gcc  ../tools/utils.h ../tools/utils.c utilsTest.c && ./a.out 
*/
#include <assert.h>
#include <stdio.h>  /*  printf */
#include "../tools/utils.h"


void listAppendTest() {
    struct List list_instance;
    initList(&list_instance);
    struct List* list = &list_instance;

    struct Item listItem_instance; 
    initItem(&listItem_instance);
    struct Item* listItem = &listItem_instance; 
    listItem->value = "hello world";
    
    struct Item listItem_instance2; 
    initItem(&listItem_instance2);
    struct Item* listItem2 = &listItem_instance2; 
    listItem2->value = "nice to meet you";
    

    assert(list->length == 0);
    listAppend(list, listItem); 
    assert(list->length == 1);
    listAppend(list, listItem2); 
    listPrint(list);
    printf("test listAppend OK\n");
}

void listInsertTest() {
    struct List list_instance;
    initList(&list_instance);
    struct List* list = &list_instance;

    struct Item listItem_instance; 
    initItem(&listItem_instance);
    struct Item* listItem = &listItem_instance; 
    listItem->value = "hello world";
    
    struct Item listItem_instance2; 
    initItem(&listItem_instance2);
    struct Item* listItem2 = &listItem_instance2; 
    listItem2->value = "nice to meet you";
    

    assert(list->length == 0);
    listInsert(list, 3, listItem); 
    assert(list->length == 1);
    listInsert(list, 0, listItem2); 
    listPrint(list);
    printf("test listAppend OK\n");

}

void mapPushTest() {
    struct Map map_instance;
    initMap(&map_instance);
    struct Map* map = &map_instance;
    
    struct Item item_instance; 
    initItem(&item_instance);
    struct Item* item = &item_instance; 
    item->key = "h";
    item->value = "hello world";
    mapPush(map, item);
    mapPrint(map);

    struct Item item_instance2; 
    initItem(&item_instance2);
    struct Item* item2 = &item_instance2; 
    item2->key = "h2";
    item2->value = "nice to meet you";
    mapPush(map, item2);
    mapPrint(map);
    printf("the value fo the key is  %s: %s, %s: %s\n",
             "h", mapGet(map, "h"), 
             "h3", mapGet(map, "h3"));
    
    
    releaseMap(map);
}


int main() {
    // listAppendTest();
    // listInsertTest();
    mapPushTest();
}
