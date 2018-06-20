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

void initListItem(struct ListItem * listItem); 
void initList(struct List * listItem); 


void listAppend(struct List* list, struct ListItem* item);
void listPrint(struct List* List);
void listInsert(struct List* list, int index, struct ListItem* item); 
struct ListItem* listGet(struct List* list, int index);
void listSet(struct List* list, int index, struct ListItem* item);
void listRemove(struct List* list, struct ListItem* item);

#define HashTableLen 100

struct MapItem {
    struct ListItem* next;
    char* value;
};

struct Map {
    struct List* table[HashTableLen];
    int table_len;
    int item_cnt;
};

void initMap(struct Map* map);
int hashCode(char* str);

void MapPush(struct Map* map, struct ListItem* item);


