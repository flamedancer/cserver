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

    struct ListItem listItem_instance; 
    initListItem(&listItem_instance);
    struct ListItem* listItem = &listItem_instance; 
    listItem->value = "hello world";
    
    struct ListItem listItem_instance2; 
    initListItem(&listItem_instance2);
    struct ListItem* listItem2 = &listItem_instance2; 
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

    struct ListItem listItem_instance; 
    initListItem(&listItem_instance);
    struct ListItem* listItem = &listItem_instance; 
    listItem->value = "hello world";
    
    struct ListItem listItem_instance2; 
    initListItem(&listItem_instance2);
    struct ListItem* listItem2 = &listItem_instance2; 
    listItem2->value = "nice to meet you";
    

    assert(list->length == 0);
    listInsert(list, 3, listItem); 
    assert(list->length == 1);
    listInsert(list, 0, listItem2); 
    listPrint(list);
    printf("test listAppend OK\n");

}


int main() {
    listAppendTest();
    listInsertTest();
}
