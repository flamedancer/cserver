
/* response.c
*/
#include <stdio.h>       /* fprintf  NULL */
#include <string.h>       /* strlen */
#include "response.h"
#include "request.h"
#include "tools/utils.h"

void initHttpResponse(struct http_response * response) {
    response->version = NULL;
    response->code = NULL;
    response->desc = NULL;
    response->headers = NULL;
    response->body = NULL;
}


void doResponse(struct http_request * request, FILE * stream) {
    struct http_response responseInstance;
    struct http_response * response = &responseInstance; 
    initHttpResponse(response);
    response->version = "HTTP/1.1";
    response->code = "200";
    response->desc = "OK";
    FILE *fileptr;
    char *buffer;
    int filelen;

    fileptr = fopen("static/home.html", "rb");         
    fseek(fileptr, 0, SEEK_END);          
    filelen = ftell(fileptr);            
    rewind(fileptr);                      
    buffer = (char *)malloc((filelen+1)*sizeof(char)); 
    fread(buffer, 1, filelen+1, fileptr); 
    // for(int i = 0; i < filelen; i++) {
    //    fread(buffer+i, 1, 1, fileptr); 
    // }
    fclose(fileptr);
    printf("buffer is %s\n",*&buffer);

    fileptr = fopen("static/pic2.jpeg", "wb");
    fprintf(fileptr, buffer);
    fclose(fileptr);



    
    // char * content = "<html>hello everyone</html>";
    // char * content = buffer;
    char content_len[25];
    // sprintf(content_len, "%lu", strlen(content));
    sprintf(content_len, "%lu", filelen);

    // printf("xxxx  %d \n", content_len);
    struct Item * item = newItem(
        "Content-Length",
        content_len
    );
    struct Map map_instance;
    initMap(&map_instance);
    response->headers = &map_instance;
   
    mapPush(response->headers, item);
    
    response->body = buffer;
    
    outputToFile(response, stream);
    
    // clean
    releaseMap(request->headers);
    free(buffer);

}


void outputToFile(struct http_response * response, FILE * stream) {
    // output version code desc
    int r = fprintf(stream, "%s %s %s \r\n",
        response->version,
        response->code,
        response->desc
    );
    // output headers
    struct Map* map = response->headers;
    struct List* list;
    struct Item* item;
    int print_item_cnt = 0;
    for(int i=0; i<map->table_len; i++) {
        list = map->table[i];
        if(list == NULL) {
           continue;
        }
        item = list->start;
        while(item != NULL) {
            fprintf(stream, "%s: %s\r\n",
                item->key,
                item->value
            );
            item = item->next;
        }
    }
    // output body
    if(response->body != NULL) {
        fprintf(stream, "\r\n%s", response->body);
    }
}
