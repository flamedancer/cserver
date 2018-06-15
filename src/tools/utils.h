/* tools/utils.h
*/
struct ListItem {
    struct ListItem* next;
    char* value;
};


struct List {
    struct ListItem* start;
    struct ListItem* end;
    int length;
};


void listAppend(struct List* list, struct ListItem* item);
void listPrint(struct List* List);
void listInsert(struct List* list, int index, struct ListItem* item); 
struct ListItem* listGet(struct List* list, int index);
void listSet(struct List* list, int index, struct ListItem* item);
void listRemove(struct List* list, struct ListItem* item);



